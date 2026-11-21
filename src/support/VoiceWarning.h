#pragma once
namespace xiv {
inline const char *voiceWarningColor(int voices) {
    return voices>=16 ? "#ff6464" : voices>=12 ? "#ffda65" : "#d8eaff";
}
}
