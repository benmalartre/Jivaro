#ifndef JVR_UI_TEXTFILTER_H
#define JVR_UI_TEXTFILTER_H

#include <functional>
#include "../ui/ui.h"

PXR_NAMESPACE_OPEN_SCOPE
/*
    The following code was copied from imgui and modified to add wildcard search
*/
struct TextFilter {
    IMGUI_API TextFilter(const char *default_filter = "");
    IMGUI_API bool Draw(const char *label = "###Filter", float width = 0.0f); // Helper calling InputText+Build
    IMGUI_API bool PassFilter(const char *text, const char *text_end = NULL) const;
    IMGUI_API void Build();
    void Clear() {
        InputBuf[0] = 0;
        Build();
    }
    bool IsActive() const { return !Filters.empty(); }

    // [Internal]
    struct TextRange {
        const char *b;
        const char *e;

        TextRange() { b = e = NULL; }
        TextRange(const char *_b, const char *_e) {
            b = _b;
            e = _e;
        }
        bool empty() const { return b == e; }
        void split(char separator, ImVector<TextRange> *out) const;
    };
    char InputBuf[256];
    ImVector<TextRange> Filters;
    int CountGrep;
    
    bool UseWildcards = false;
    std::function<const char *(const char *, const char *, const char *, const char *)> PatternMatchFunc;

};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_TEXTFILTER_H