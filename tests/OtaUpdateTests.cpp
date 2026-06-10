#include "ota/OtaUpdate.hpp"

#include <cassert>
#include <sstream>
#include <string>

namespace {

bool contains(const std::string& value, const std::string& needle) {
  return value.find(needle) != std::string::npos;
}

ota::UpdatePlan planLinux(ota::UpdateManifest manifest = ota::demoLinuxManifest(),
                          ota::DeviceIdentity device = ota::demoLinuxDevice(),
                          ota::UpdateSlot active = ota::activeLinuxSlot(),
                          ota::UpdateSlot inactive = ota::inactiveLinuxSlot()) {
  ota::SimulatedManifestVerifier verifier;
  ota::OtaUpdatePlanner planner(ota::OtaPolicy{}, verifier);
  return planner.plan(device, active, inactive, manifest);
}

void acceptsLinuxRootfsUpdate() {
  const auto plan = planLinux();

  assert(plan.accepted);
  assert(plan.action == ota::PlanAction::InstallToInactiveSlot);
  assert(plan.targetSlot == "rootfs_b");
  assert(contains(plan.reason, "2026.06.0"));
}

void acceptsMcuFirmwareUpdate() {
  ota::SimulatedManifestVerifier verifier;
  ota::OtaUpdatePlanner planner(ota::OtaPolicy{}, verifier);
  ota::UpdateSlot slotA{"slot_a", ota::SlotRole::Active, "1.4.0", true, true, 0U};
  ota::UpdateSlot slotB{"slot_b", ota::SlotRole::Inactive, "1.3.9", true, true, 0U};

  const auto plan =
      planner.plan(ota::demoMcuDevice(), slotA, slotB, ota::demoMcuManifest());

  assert(plan.accepted);
  assert(plan.targetSlot == "slot_b");
}

void rejectsLinuxUpdateWithoutRootfsBundle() {
  auto manifest = ota::demoLinuxManifest();
  manifest.linuxRootfs = false;

  const auto plan = planLinux(manifest);

  assert(!plan.accepted);
  assert(contains(plan.reason, "rootfs"));
}

void rejectsDeviceOutsideRollout() {
  auto manifest = ota::demoLinuxManifest();
  manifest.rolloutPercent = 10U;

  const auto plan = planLinux(manifest);

  assert(!plan.accepted);
  assert(contains(plan.reason, "outside staged rollout"));
}

void rejectsInsecureArtifactUri() {
  auto manifest = ota::demoLinuxManifest();
  manifest.artifactUri = "http://updates.example.com/rootfs.raucb";

  const auto plan = planLinux(manifest);

  assert(!plan.accepted);
  assert(contains(plan.reason, "HTTPS"));
}

void rollsBackFailedTrialBoot() {
  auto active = ota::activeLinuxSlot();
  active.confirmed = false;
  active.bootAttempts = 3U;

  const auto plan =
      planLinux(ota::demoLinuxManifest(), ota::demoLinuxDevice(), active, ota::inactiveLinuxSlot());

  assert(plan.accepted);
  assert(plan.action == ota::PlanAction::RollbackToPreviousSlot);
  assert(plan.targetSlot == "rootfs_b");
}

void reporterIncludesLinuxEvidence() {
  const auto plan = planLinux();
  std::ostringstream output;
  ota::TextPlanReporter reporter(output);
  reporter.publish(plan);

  assert(contains(output.str(), "INSTALL_TO_INACTIVE_SLOT"));
  assert(contains(output.str(), "artifact-policy"));
}

}  // namespace

int main() {
  acceptsLinuxRootfsUpdate();
  acceptsMcuFirmwareUpdate();
  rejectsLinuxUpdateWithoutRootfsBundle();
  rejectsDeviceOutsideRollout();
  rejectsInsecureArtifactUri();
  rollsBackFailedTrialBoot();
  reporterIncludesLinuxEvidence();
  return 0;
}
