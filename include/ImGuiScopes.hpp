#ifndef IMGUI_SCOPES_HPP_INCLUDED
#define IMGUI_SCOPES_HPP_INCLUDED

#include <imgui.h>

// Scope guards for Dear ImGui's paired calls. Each guard calls the matching
// End/Pop in its destructor, and only when ImGui requires it, so early
// returns and exceptions cannot leave ImGui's stacks unbalanced.
//
//     if (const ui::Window window{"Inspector"}) {
//         const ui::Id id{robot.id};
//         if (const ui::Table table{"needs", 2}) { ... }
//     }
//
// Guards that open something optional (tables, tree nodes, popups, menus,
// combos, tab bars) convert to true when it is open; use them in an `if`.
// Guards are neither copyable nor movable, and their constructors are
// [[nodiscard]]: a guard written as a bare temporary would close at once.
namespace ui {

class Scope {
public:
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
    Scope(Scope&&) = delete;
    Scope& operator=(Scope&&) = delete;

protected:
    Scope() = default;
    ~Scope() = default;
};

// ImGui::Begin / End. End is always required, even when collapsed.
class Window : Scope {
public:
    [[nodiscard]] explicit Window(const char* name, bool* open = nullptr,
                                  ImGuiWindowFlags flags = 0)
        : visible_(ImGui::Begin(name, open, flags)) {}
    ~Window() { ImGui::End(); }
    // False when collapsed or clipped: skip drawing its contents.
    explicit operator bool() const noexcept { return visible_; }

private:
    bool visible_;
};

// ImGui::BeginChild / EndChild. EndChild is always required.
class Child : Scope {
public:
    [[nodiscard]] explicit Child(const char* id, ImVec2 size = {0.0F, 0.0F},
                                 ImGuiChildFlags childFlags = 0,
                                 ImGuiWindowFlags windowFlags = 0)
        : visible_(ImGui::BeginChild(id, size, childFlags, windowFlags)) {}
    ~Child() { ImGui::EndChild(); }
    explicit operator bool() const noexcept { return visible_; }

private:
    bool visible_;
};

// ImGui::BeginTable / EndTable, ended only if it began.
class Table : Scope {
public:
    [[nodiscard]] Table(const char* id, int columns, ImGuiTableFlags flags = 0,
                        ImVec2 outerSize = {0.0F, 0.0F}, float innerWidth = 0.0F)
        : open_(ImGui::BeginTable(id, columns, flags, outerSize, innerWidth)) {}
    ~Table() {
        if (open_) {
            ImGui::EndTable();
        }
    }
    explicit operator bool() const noexcept { return open_; }

private:
    bool open_;
};

// ImGui::TreeNodeEx / TreePop, popped only if open and pushed.
class TreeNode : Scope {
public:
    [[nodiscard]] explicit TreeNode(const char* label,
                                    ImGuiTreeNodeFlags flags = 0)
        : open_(ImGui::TreeNodeEx(label, flags)),
          pushed_(open_ && (flags & ImGuiTreeNodeFlags_NoTreePushOnOpen) == 0) {
    }
    ~TreeNode() {
        if (pushed_) {
            ImGui::TreePop();
        }
    }
    explicit operator bool() const noexcept { return open_; }

private:
    bool open_;
    bool pushed_;
};

// ImGui::BeginCombo / EndCombo, ended only if open.
class Combo : Scope {
public:
    [[nodiscard]] Combo(const char* label, const char* preview,
                        ImGuiComboFlags flags = 0)
        : open_(ImGui::BeginCombo(label, preview, flags)) {}
    ~Combo() {
        if (open_) {
            ImGui::EndCombo();
        }
    }
    explicit operator bool() const noexcept { return open_; }

private:
    bool open_;
};

// ImGui::BeginPopup / EndPopup, ended only if open.
class Popup : Scope {
public:
    [[nodiscard]] explicit Popup(const char* id, ImGuiWindowFlags flags = 0)
        : open_(ImGui::BeginPopup(id, flags)) {}
    ~Popup() {
        if (open_) {
            ImGui::EndPopup();
        }
    }
    explicit operator bool() const noexcept { return open_; }

private:
    bool open_;
};

// ImGui::BeginPopupModal / EndPopup, ended only if open.
class PopupModal : Scope {
public:
    [[nodiscard]] explicit PopupModal(const char* name, bool* open = nullptr,
                                      ImGuiWindowFlags flags = 0)
        : open_(ImGui::BeginPopupModal(name, open, flags)) {}
    ~PopupModal() {
        if (open_) {
            ImGui::EndPopup();
        }
    }
    explicit operator bool() const noexcept { return open_; }

private:
    bool open_;
};

// ImGui::BeginItemTooltip / EndTooltip for the last item, ended only if
// shown.
class ItemTooltip : Scope {
public:
    [[nodiscard]] ItemTooltip() : open_(ImGui::BeginItemTooltip()) {}
    ~ItemTooltip() {
        if (open_) {
            ImGui::EndTooltip();
        }
    }
    explicit operator bool() const noexcept { return open_; }

private:
    bool open_;
};

// ImGui::BeginMenuBar / EndMenuBar, ended only if it began.
class MenuBar : Scope {
public:
    [[nodiscard]] MenuBar() : open_(ImGui::BeginMenuBar()) {}
    ~MenuBar() {
        if (open_) {
            ImGui::EndMenuBar();
        }
    }
    explicit operator bool() const noexcept { return open_; }

private:
    bool open_;
};

// ImGui::BeginMenu / EndMenu, ended only if open.
class Menu : Scope {
public:
    [[nodiscard]] explicit Menu(const char* label, bool enabled = true)
        : open_(ImGui::BeginMenu(label, enabled)) {}
    ~Menu() {
        if (open_) {
            ImGui::EndMenu();
        }
    }
    explicit operator bool() const noexcept { return open_; }

private:
    bool open_;
};

// ImGui::BeginTabBar / EndTabBar, ended only if it began.
class TabBar : Scope {
public:
    [[nodiscard]] explicit TabBar(const char* id, ImGuiTabBarFlags flags = 0)
        : open_(ImGui::BeginTabBar(id, flags)) {}
    ~TabBar() {
        if (open_) {
            ImGui::EndTabBar();
        }
    }
    explicit operator bool() const noexcept { return open_; }

private:
    bool open_;
};

// ImGui::BeginTabItem / EndTabItem, ended only if selected.
class TabItem : Scope {
public:
    [[nodiscard]] explicit TabItem(const char* label, bool* open = nullptr,
                                   ImGuiTabItemFlags flags = 0)
        : open_(ImGui::BeginTabItem(label, open, flags)) {}
    ~TabItem() {
        if (open_) {
            ImGui::EndTabItem();
        }
    }
    explicit operator bool() const noexcept { return open_; }

private:
    bool open_;
};

// ImGui::PushID / PopID.
class Id : Scope {
public:
    [[nodiscard]] explicit Id(const char* id) { ImGui::PushID(id); }
    [[nodiscard]] explicit Id(const void* id) { ImGui::PushID(id); }
    [[nodiscard]] explicit Id(int id) { ImGui::PushID(id); }
    ~Id() { ImGui::PopID(); }
};

// ImGui::PushStyleColor / PopStyleColor.
class StyleColor : Scope {
public:
    [[nodiscard]] StyleColor(ImGuiCol index, const ImVec4& color) {
        ImGui::PushStyleColor(index, color);
    }
    [[nodiscard]] StyleColor(ImGuiCol index, ImU32 color) {
        ImGui::PushStyleColor(index, color);
    }
    ~StyleColor() { ImGui::PopStyleColor(); }
};

// ImGui::PushStyleVar / PopStyleVar.
class StyleVar : Scope {
public:
    [[nodiscard]] StyleVar(ImGuiStyleVar index, float value) {
        ImGui::PushStyleVar(index, value);
    }
    [[nodiscard]] StyleVar(ImGuiStyleVar index, const ImVec2& value) {
        ImGui::PushStyleVar(index, value);
    }
    ~StyleVar() { ImGui::PopStyleVar(); }
};

// ImGui::PushItemWidth / PopItemWidth.
class ItemWidth : Scope {
public:
    [[nodiscard]] explicit ItemWidth(float width) { ImGui::PushItemWidth(width); }
    ~ItemWidth() { ImGui::PopItemWidth(); }
};

// ImGui::BeginDisabled / EndDisabled. Always ended.
class Disabled : Scope {
public:
    [[nodiscard]] explicit Disabled(bool disabled = true) {
        ImGui::BeginDisabled(disabled);
    }
    ~Disabled() { ImGui::EndDisabled(); }
};

// ImGui::BeginGroup / EndGroup.
class Group : Scope {
public:
    [[nodiscard]] Group() { ImGui::BeginGroup(); }
    ~Group() { ImGui::EndGroup(); }
};

// ImGui::Indent / Unindent.
class Indent : Scope {
public:
    [[nodiscard]] explicit Indent(float width = 0.0F) : width_(width) {
        ImGui::Indent(width_);
    }
    ~Indent() { ImGui::Unindent(width_); }

private:
    float width_;
};

}  // namespace ui

#endif  // IMGUI_SCOPES_HPP_INCLUDED
