#ifndef OTA_OTA_UPDATE_HPP_
#define OTA_OTA_UPDATE_HPP_

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

namespace ota {

enum class TargetKind {
  Microcontroller,
  EmbeddedLinux
};

enum class SlotRole {
  Active,
  Inactive
};

enum class PlanAction {
  InstallToInactiveSlot,
  RebootToPendingSlot,
  RollbackToPreviousSlot,
  StayOnCurrent,
  Reject
};

enum class CheckStatus {
  Pass,
  Fail
};

std::string toString(TargetKind target);
std::string toString(PlanAction action);
std::string toString(CheckStatus status);

struct DeviceIdentity {
  std::string deviceId;
  TargetKind target{TargetKind::Microcontroller};
  std::string hardwareRevision;
  std::string currentVersion;
  std::uint32_t rollbackCounter{};
  std::uint8_t rolloutBucket{};
};

struct UpdateSlot {
  std::string name;
  SlotRole role{SlotRole::Inactive};
  std::string version;
  bool bootable{};
  bool confirmed{};
  std::uint8_t bootAttempts{};
};

struct UpdateManifest {
  std::string version;
  TargetKind target{TargetKind::Microcontroller};
  std::string hardwareRevision;
  std::string artifactUri;
  std::string sha256;
  std::string signature;
  std::uint32_t rollbackCounter{};
  std::uint8_t rolloutPercent{100};
  std::size_t artifactBytes{};
  bool signaturePresent{};
  bool linuxRootfs{};
};

struct OtaPolicy {
  std::uint8_t maxBootAttempts{3};
  std::size_t maxMcuArtifactBytes{768U * 1024U};
  std::size_t maxLinuxArtifactBytes{256U * 1024U * 1024U};
  bool requireTls{true};
  bool requireLinuxRootfsForLinux{true};
};

struct PlanCheck {
  std::string step;
  CheckStatus status{CheckStatus::Fail};
  std::string detail;
};

struct UpdatePlan {
  bool accepted{};
  PlanAction action{PlanAction::Reject};
  std::string targetSlot;
  std::string reason;
  std::vector<PlanCheck> checks;
};

class IManifestVerifier {
 public:
  virtual ~IManifestVerifier() = default;
  virtual bool verify(const UpdateManifest& manifest,
                      std::string& reason) const = 0;
};

class OtaUpdatePlanner {
 public:
  OtaUpdatePlanner(OtaPolicy policy, const IManifestVerifier& verifier);

  UpdatePlan plan(const DeviceIdentity& device,
                  const UpdateSlot& slotA,
                  const UpdateSlot& slotB,
                  const UpdateManifest& manifest) const;

 private:
  OtaPolicy policy_;
  const IManifestVerifier& verifier_;
};

class SimulatedManifestVerifier final : public IManifestVerifier {
 public:
  bool verify(const UpdateManifest& manifest, std::string& reason) const override;
};

class TextPlanReporter {
 public:
  explicit TextPlanReporter(std::ostream& stream);

  void publish(const UpdatePlan& plan) const;

 private:
  std::ostream& stream_;
};

DeviceIdentity demoLinuxDevice();
UpdateSlot activeLinuxSlot();
UpdateSlot inactiveLinuxSlot();
UpdateManifest demoLinuxManifest();
DeviceIdentity demoMcuDevice();
UpdateManifest demoMcuManifest();

}  // namespace ota

#endif  // OTA_OTA_UPDATE_HPP_
