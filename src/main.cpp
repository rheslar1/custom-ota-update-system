#include "ota/OtaUpdate.hpp"

#include <iostream>

int main() {
  ota::SimulatedManifestVerifier verifier;
  ota::OtaUpdatePlanner planner(ota::OtaPolicy{}, verifier);
  ota::TextPlanReporter reporter(std::cout);

  std::cout << "Custom OTA Update System\n";
  std::cout << "Targets: MCU dual-partition firmware and Embedded Linux A/B rootfs\n\n";

  const auto linuxPlan = planner.plan(ota::demoLinuxDevice(),
                                      ota::activeLinuxSlot(),
                                      ota::inactiveLinuxSlot(),
                                      ota::demoLinuxManifest());
  reporter.publish(linuxPlan);
  std::cout << '\n';

  ota::UpdateSlot mcuA{"slot_a", ota::SlotRole::Active, "1.4.0", true, true, 0U};
  ota::UpdateSlot mcuB{"slot_b", ota::SlotRole::Inactive, "1.3.9", true, true, 0U};
  const auto mcuPlan =
      planner.plan(ota::demoMcuDevice(), mcuA, mcuB, ota::demoMcuManifest());
  reporter.publish(mcuPlan);

  return linuxPlan.accepted && mcuPlan.accepted ? 0 : 1;
}
