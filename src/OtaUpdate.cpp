#include "ota/OtaUpdate.hpp"

#include <algorithm>
#include <cctype>
#include <ostream>
#include <string_view>
#include <utility>

namespace ota {
namespace {

bool startsWith(std::string_view value, std::string_view prefix) {
  return value.substr(0U, prefix.size()) == prefix;
}

bool isHexDigest(std::string_view value) {
  return value.size() == 64U &&
         std::all_of(value.begin(), value.end(), [](const char item) {
           return std::isxdigit(static_cast<unsigned char>(item)) != 0;
         });
}

void addCheck(UpdatePlan& plan,
              std::string step,
              const bool passed,
              std::string detail) {
  plan.checks.push_back(
      PlanCheck{std::move(step),
                passed ? CheckStatus::Pass : CheckStatus::Fail,
                std::move(detail)});
}

const UpdateSlot& activeSlot(const UpdateSlot& slotA, const UpdateSlot& slotB) {
  return slotA.role == SlotRole::Active ? slotA : slotB;
}

const UpdateSlot& inactiveSlot(const UpdateSlot& slotA, const UpdateSlot& slotB) {
  return slotA.role == SlotRole::Inactive ? slotA : slotB;
}

bool slotRolesValid(const UpdateSlot& slotA,
                    const UpdateSlot& slotB,
                    std::string& reason) {
  if (slotA.name.empty() || slotB.name.empty() || slotA.name == slotB.name) {
    reason = "slots must have distinct names";
    return false;
  }

  if (slotA.role == slotB.role) {
    reason = "exactly one slot must be active";
    return false;
  }

  reason = "active slot is " + activeSlot(slotA, slotB).name +
           ", inactive slot is " + inactiveSlot(slotA, slotB).name;
  return true;
}

bool compatibleTarget(const DeviceIdentity& device,
                      const UpdateManifest& manifest,
                      std::string& reason) {
  if (device.target != manifest.target) {
    reason = "manifest target does not match device";
    return false;
  }

  if (device.hardwareRevision != manifest.hardwareRevision) {
    reason = "manifest hardware " + manifest.hardwareRevision +
             " does not match device " + device.hardwareRevision;
    return false;
  }

  reason = toString(device.target) + " target and hardware revision accepted";
  return true;
}

bool rolloutAllowed(const DeviceIdentity& device,
                    const UpdateManifest& manifest,
                    std::string& reason) {
  if (manifest.rolloutPercent == 0U || manifest.rolloutPercent > 100U) {
    reason = "rollout percent must be 1-100";
    return false;
  }

  const bool allowed = device.rolloutBucket < manifest.rolloutPercent;
  reason = allowed ? "device bucket inside staged rollout"
                   : "device bucket outside staged rollout";
  return allowed;
}

bool artifactAllowed(const OtaPolicy& policy,
                     const DeviceIdentity& device,
                     const UpdateManifest& manifest,
                     std::string& reason) {
  if (policy.requireTls && !startsWith(manifest.artifactUri, "https://")) {
    reason = "artifact URI must use HTTPS";
    return false;
  }

  const std::size_t limit = device.target == TargetKind::EmbeddedLinux
                                ? policy.maxLinuxArtifactBytes
                                : policy.maxMcuArtifactBytes;
  if (manifest.artifactBytes == 0U || manifest.artifactBytes > limit) {
    reason = "artifact size exceeds target budget";
    return false;
  }

  if (device.target == TargetKind::EmbeddedLinux &&
      policy.requireLinuxRootfsForLinux && !manifest.linuxRootfs) {
    reason = "Linux OTA must install a rootfs or bundle artifact";
    return false;
  }

  reason = "artifact URI and size accepted";
  return true;
}

bool rollbackAllowed(const DeviceIdentity& device,
                     const UpdateManifest& manifest,
                     std::string& reason) {
  if (manifest.rollbackCounter < device.rollbackCounter) {
    reason = "manifest rollback counter is older than device";
    return false;
  }
  reason = "rollback counter accepted";
  return true;
}

bool needsRollback(const UpdateSlot& active, const OtaPolicy& policy) {
  return active.bootable && !active.confirmed &&
         active.bootAttempts >= policy.maxBootAttempts;
}

}  // namespace

std::string toString(const TargetKind target) {
  return target == TargetKind::EmbeddedLinux ? "EmbeddedLinux" : "Microcontroller";
}

std::string toString(const PlanAction action) {
  switch (action) {
    case PlanAction::InstallToInactiveSlot:
      return "INSTALL_TO_INACTIVE_SLOT";
    case PlanAction::RebootToPendingSlot:
      return "REBOOT_TO_PENDING_SLOT";
    case PlanAction::RollbackToPreviousSlot:
      return "ROLLBACK_TO_PREVIOUS_SLOT";
    case PlanAction::StayOnCurrent:
      return "STAY_ON_CURRENT";
    case PlanAction::Reject:
      return "REJECT";
  }
  return "UNKNOWN";
}

std::string toString(const CheckStatus status) {
  return status == CheckStatus::Pass ? "PASS" : "FAIL";
}

OtaUpdatePlanner::OtaUpdatePlanner(OtaPolicy policy,
                                   const IManifestVerifier& verifier)
    : policy_(policy), verifier_(verifier) {}

UpdatePlan OtaUpdatePlanner::plan(const DeviceIdentity& device,
                                  const UpdateSlot& slotA,
                                  const UpdateSlot& slotB,
                                  const UpdateManifest& manifest) const {
  UpdatePlan plan;

  auto finish = [&](const bool accepted,
                    const PlanAction action,
                    std::string targetSlot,
                    std::string reason) {
    plan.accepted = accepted;
    plan.action = action;
    plan.targetSlot = std::move(targetSlot);
    plan.reason = std::move(reason);
    return plan;
  };

  std::string reason;
  const bool slotsOk = slotRolesValid(slotA, slotB, reason);
  addCheck(plan, "slot-layout", slotsOk, reason);
  if (!slotsOk) {
    return finish(false, PlanAction::Reject, "", reason);
  }

  const auto& active = activeSlot(slotA, slotB);
  const auto& inactive = inactiveSlot(slotA, slotB);
  if (needsRollback(active, policy_)) {
    addCheck(plan,
             "rollback-health",
             true,
             "active slot failed trial boot attempts");
    return finish(true,
                  PlanAction::RollbackToPreviousSlot,
                  inactive.name,
                  "rollback to previous confirmed slot");
  }
  addCheck(plan, "rollback-health", true, "active slot does not require rollback");

  if (manifest.version == device.currentVersion) {
    addCheck(plan, "version-gate", true, "device already running manifest version");
    return finish(true, PlanAction::StayOnCurrent, active.name, "already current");
  }
  addCheck(plan, "version-gate", true, "manifest version is newer candidate");

  const bool targetOk = compatibleTarget(device, manifest, reason);
  addCheck(plan, "target-compatibility", targetOk, reason);
  if (!targetOk) {
    return finish(false, PlanAction::Reject, "", reason);
  }

  const bool rolloutOk = rolloutAllowed(device, manifest, reason);
  addCheck(plan, "staged-rollout", rolloutOk, reason);
  if (!rolloutOk) {
    return finish(false, PlanAction::Reject, "", reason);
  }

  const bool artifactOk = artifactAllowed(policy_, device, manifest, reason);
  addCheck(plan, "artifact-policy", artifactOk, reason);
  if (!artifactOk) {
    return finish(false, PlanAction::Reject, "", reason);
  }

  const bool signatureOk = verifier_.verify(manifest, reason);
  addCheck(plan, "signature-policy", signatureOk, reason);
  if (!signatureOk) {
    return finish(false, PlanAction::Reject, "", reason);
  }

  const bool rollbackOk = rollbackAllowed(device, manifest, reason);
  addCheck(plan, "rollback-counter", rollbackOk, reason);
  if (!rollbackOk) {
    return finish(false, PlanAction::Reject, "", reason);
  }

  return finish(true,
                PlanAction::InstallToInactiveSlot,
                inactive.name,
                "install " + manifest.version + " to " + inactive.name);
}

bool SimulatedManifestVerifier::verify(const UpdateManifest& manifest,
                                       std::string& reason) const {
  if (!manifest.signaturePresent) {
    reason = "signature missing";
    return false;
  }

  if (!isHexDigest(manifest.sha256)) {
    reason = "sha256 digest must be 64 hex characters";
    return false;
  }

  if (manifest.signature != "ECDSA-P256:VALID" &&
      manifest.signature != "RSA3072:VALID") {
    reason = "signature verification failed";
    return false;
  }

  reason = "manifest signature and digest accepted";
  return true;
}

TextPlanReporter::TextPlanReporter(std::ostream& stream) : stream_(stream) {}

void TextPlanReporter::publish(const UpdatePlan& plan) const {
  stream_ << "ota=" << (plan.accepted ? "PASS" : "FAIL")
          << " action=" << toString(plan.action) << " target=\""
          << plan.targetSlot << "\" reason=\"" << plan.reason << "\"\n";
  for (const auto& check : plan.checks) {
    stream_ << "  [" << toString(check.status) << "] " << check.step << ": "
            << check.detail << '\n';
  }
}

DeviceIdentity demoLinuxDevice() {
  return DeviceIdentity{"bems-linux-gw-001",
                        TargetKind::EmbeddedLinux,
                        "imx93-rev-b",
                        "2026.05.0",
                        100U,
                        23U};
}

UpdateSlot activeLinuxSlot() {
  return UpdateSlot{"rootfs_a", SlotRole::Active, "2026.05.0", true, true, 0U};
}

UpdateSlot inactiveLinuxSlot() {
  return UpdateSlot{"rootfs_b", SlotRole::Inactive, "2026.04.1", true, true, 0U};
}

UpdateManifest demoLinuxManifest() {
  return UpdateManifest{
      "2026.06.0",
      TargetKind::EmbeddedLinux,
      "imx93-rev-b",
      "https://updates.example.com/bems/rootfs-2026.06.0.raucb",
      "3b7e1f0c9d4a5b682817263544e5f60718c9aabbccddeeff0011223344556677",
      "RSA3072:VALID",
      101U,
      50U,
      96U * 1024U * 1024U,
      true,
      true};
}

DeviceIdentity demoMcuDevice() {
  return DeviceIdentity{
      "stm32-node-017", TargetKind::Microcontroller, "stm32-rev-c", "1.4.0", 18U, 7U};
}

UpdateManifest demoMcuManifest() {
  return UpdateManifest{
      "1.5.0",
      TargetKind::Microcontroller,
      "stm32-rev-c",
      "https://updates.example.com/mcu/app-1.5.0.bin",
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
      "ECDSA-P256:VALID",
      19U,
      25U,
      384U * 1024U,
      true,
      false};
}

}  // namespace ota
