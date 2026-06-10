#include <array>
#include <cassert>
#include <string_view>

class IReadinessRule {
 public:
  virtual ~IReadinessRule() = default;
  virtual bool passes(std::string_view evidenceTarget) const = 0;
};

class RequiredEvidenceRule final : public IReadinessRule {
 public:
  bool passes(std::string_view evidenceTarget) const override {
    return !evidenceTarget.empty();
  }
};

struct ProjectProfile {
  std::string_view title;
  std::string_view summary;
  std::string_view evidenceTarget;
  std::array<std::string_view, 9> tags;
};

constexpr ProjectProfile profile{
  "Custom OTA Update System",
  "Secure dual-partition firmware update flow for ESP32 or STM32 with signed image verification, staged rollouts, and automatic fallback.",
  "Production deployment pipelines, secure update handling, flash layout discipline, and field recovery behavior.",
  {
    "C++17",
    "C++ Design Patterns",
    "SOLID",
    "ESP32/STM32",
    "Dual partition",
    "MCUboot",
    "ECDSA",
    "TLS",
    "Rollback logic"
  }
};

int main() {
  const RequiredEvidenceRule rule;
  assert(!profile.title.empty());
  assert(!profile.summary.empty());
  assert(rule.passes(profile.evidenceTarget));
  assert(profile.tags[0] == "C++17");
  return 0;
}
