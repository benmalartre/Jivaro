#include "../ui/textFilter.h"

JVR_NAMESPACE_OPEN_SCOPE
//
// Using a more performant algorithm for wildcard matching:
// https://en.wikipedia.org/wiki/Matching_wildcards
// 
#define USE_KRAUSS_WILDCARDS_ALGO 1

#ifdef USE_KRAUSS_WILDCARDS_ALGO
#include "../utils/wildchar.h"
const char *ImStrWildcards(const char *haystack, const char *haystack_end, const char *needle, const char *needle_end) {
    return FastWildComparePortable(needle, haystack) ? haystack : nullptr;
}
#else
const char *ImStrWildcards(const char *haystack, const char *haystack_end, const char *needle, const char *needle_end) {
    if (!needle_end)
        needle_end = needle + strlen(needle);
    if (!haystack_end)
        haystack_end = haystack + strlen(haystack);

    const size_t haystack_size = std::distance(haystack, haystack_end);
    const size_t needle_size = std::distance(needle, needle_end);

    if (!needle_size)
        return nullptr;
    if (!haystack_size)
        return nullptr;

    // Memory optimisation: using only the current and previous line
    bool dpbuf[2][(needle_size + 1)];

    // Init empty character
    dpbuf[0][0] = true;
    dpbuf[1][0] = false;
    for (int i = 1; i <= needle_size; ++i) {
        if (needle[i - 1] == '*')
            dpbuf[0][i] = dpbuf[0][i - 1];
        else
            dpbuf[0][i] = false;
    }

    // Fill dp array
    for (int j = 1; j <= haystack_size; ++j) {
        const int jmod2 = j & 1;
        const int jm1mod2 = (j - 1) & 1;
        dpbuf[jmod2][0] = false;
        for (int i = 1; i <= needle_size; ++i) {
            if (haystack[j - 1] == needle[i - 1] || needle[i - 1] == '?') {
                dpbuf[jmod2][i] = dpbuf[jm1mod2][i - 1];
            } else if (needle[i - 1] == '*') {
                dpbuf[jmod2][i] = dpbuf[jm1mod2][i] || dpbuf[jmod2][i - 1];
            } else {
                dpbuf[jmod2][i] = false;
            }
        }
    }

    return dpbuf[haystack_size % 2][needle_size] ? haystack : nullptr;
}
#endif

TextFilter::TextFilter(const char *default_filter) {
    if (default_filter) {
        ImStrncpy(InputBuf, default_filter, IM_ARRAYSIZE(InputBuf));
        Build();
    } else {
        InputBuf[0] = 0;
        CountGrep = 0;
    }
}

bool TextFilter::Draw(const char *label, float width) {
    if (width != 0.0f)
        ImGui::SetNextItemWidth(width);
    bool value_changed = ImGui::InputTextWithHint(label, "Search", InputBuf, IM_ARRAYSIZE(InputBuf));
    ImGui::SameLine();
    if (ImGui::Checkbox("Use wildcard", &UseWildcards) || value_changed)
        Build();
    return value_changed;
}

void TextFilter::TextRange::split(char separator, ImVector<TextRange> *out) const {
    out->resize(0);
    const char *wb = b;
    const char *we = wb;
    while (we < e) {
        if (*we == separator) {
            out->push_back(TextRange(wb, we));
            wb = we + 1;
        }
        we++;
    }
    if (wb != we)
        out->push_back(TextRange(wb, we));
}

void TextFilter::Build() {
    
    PatternMatchFunc = UseWildcards ? ImStrWildcards : ImStristr;
    
    Filters.resize(0);
    TextRange input_range(InputBuf, InputBuf + strlen(InputBuf));
    input_range.split(',', &Filters);

    CountGrep = 0;
    for (int i = 0; i != Filters.Size; i++) {
        TextRange &f = Filters[i];
        while (f.b < f.e && ImCharIsBlankA(f.b[0]))
            f.b++;
        while (f.e > f.b && ImCharIsBlankA(f.e[-1]))
            f.e--;
        if (f.empty())
            continue;
        if (Filters[i].b[0] != '-')
            CountGrep += 1;
    }
}

bool TextFilter::PassFilter(const char *text, const char *text_end) const {
    if (Filters.empty())
        return true;

    if (text == NULL)
        text = "";

    for (int i = 0; i != Filters.Size; i++) {
        const TextRange &f = Filters[i];
        if (f.empty())
            continue;
        if (f.b[0] == '-') {
            // Subtract
            if (PatternMatchFunc(text, text_end, f.b + 1, f.e) != NULL)
                return false;
        } else {
            // Grep
            if (PatternMatchFunc(text, text_end, f.b, f.e) != NULL)
                return true;
        }
    }

    // Implicit * grep
    if (CountGrep == 0)
        return true;

    return false;
}

JVR_NAMESPACE_CLOSE_SCOPE
