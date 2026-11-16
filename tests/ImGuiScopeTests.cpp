#include <gtest/gtest.h>
#include <imgui.h>
#include <imgui_impl_null.h>
#include <imgui_internal.h>

#include <stdexcept>

#include "ImGuiScopes.hpp"

namespace {

// The parts of ImGui's state that unbalanced Begin/End or Push/Pop calls
// leave behind.
struct Stacks {
    int windows{};
    int colors{};
    int styleVars{};
    int groups{};
    int popups{};
    int itemWidths{};
    int ids{};
    bool inTable{};

    bool operator==(const Stacks&) const = default;
};

Stacks Capture() {
    const ImGuiContext& context = *ImGui::GetCurrentContext();
    const ImGuiWindow* window = context.CurrentWindow;
    return {.windows = context.CurrentWindowStack.Size,
            .colors = context.ColorStack.Size,
            .styleVars = context.StyleVarStack.Size,
            .groups = context.GroupStack.Size,
            .popups = context.BeginPopupStack.Size,
            .itemWidths = window != nullptr ? window->DC.ItemWidthStack.Size : 0,
            .ids = window != nullptr ? window->IDStack.Size : 0,
            .inTable = context.CurrentTable != nullptr};
}

// A headless ImGui frame on the null backend; ending it would assert if any
// scope were left open.
class ImGuiScopes : public testing::Test {
protected:
    void SetUp() override {
        ImGui::CreateContext();
        ImGui::GetIO().IniFilename = nullptr;
        ASSERT_TRUE(ImGui_ImplNull_Init());
        ImGui_ImplNull_NewFrame();
        ImGui::NewFrame();
        outside_ = Capture();
    }

    void TearDown() override {
        EXPECT_EQ(Capture(), outside_);
        ImGui::Render();
        ImGui_ImplNullRender_RenderDrawData(ImGui::GetDrawData());
        ImGui_ImplNull_Shutdown();
        ImGui::DestroyContext();
    }

    // Ends the current frame and starts the next one.
    void NextFrame() {
        EXPECT_EQ(Capture(), outside_);
        ImGui::Render();
        ImGui_ImplNullRender_RenderDrawData(ImGui::GetDrawData());
        ImGui_ImplNull_NewFrame();
        ImGui::NewFrame();
    }

    Stacks outside_;
};

}  // namespace

TEST_F(ImGuiScopes, WindowEndsEvenWhenCollapsed) {
    // A new window still reports itself visible on its first frame.
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_Always);
    { const ui::Window window{"Collapsed"}; }
    NextFrame();
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_Always);
    {
        const ui::Window window{"Collapsed"};
        EXPECT_FALSE(window);
        EXPECT_EQ(Capture().windows, outside_.windows + 1);
    }
    EXPECT_EQ(Capture(), outside_);
    {
        const ui::Window window{"Open"};
        EXPECT_TRUE(window);
        const ui::Child child{"child", {50.0F, 50.0F}};
        EXPECT_EQ(Capture().windows, outside_.windows + 2);
    }
}

TEST_F(ImGuiScopes, OptionalScopesCloseOnlyWhatOpened) {
    const ui::Window window{"Optional"};
    const Stacks inWindow = Capture();
    {
        const ui::TreeNode closed{"closed"};
        EXPECT_FALSE(closed);
    }
    ImGui::SetNextItemOpen(true);
    {
        const ui::TreeNode open{"open"};
        EXPECT_TRUE(open);
        EXPECT_EQ(Capture().ids, inWindow.ids + 1);
    }
    ImGui::SetNextItemOpen(true);
    {
        const ui::TreeNode leaf{"leaf", ImGuiTreeNodeFlags_NoTreePushOnOpen};
        EXPECT_TRUE(leaf);
        EXPECT_EQ(Capture().ids, inWindow.ids);
    }
    {
        const ui::Combo combo{"combo", "preview"};
        EXPECT_FALSE(combo);
        const ui::Popup popup{"never opened"};
        EXPECT_FALSE(popup);
    }
    {
        const ui::Table table{"table", 2};
        EXPECT_TRUE(table);
        EXPECT_TRUE(Capture().inTable);
    }
    {
        const ui::TabBar tabs{"tabs"};
        ASSERT_TRUE(tabs);
        const ui::TabItem first{"First"};
        EXPECT_TRUE(first);
    }
    EXPECT_EQ(Capture(), inWindow);
}

TEST_F(ImGuiScopes, EarlyReturnUnwindsEveryScope) {
    const auto draw = [] {
        const ui::Window window{"Early"};
        const ui::Id id{7};
        const ui::StyleColor color{ImGuiCol_Text, ImVec4{1.0F, 0.0F, 0.0F, 1.0F}};
        const ui::StyleVar alpha{ImGuiStyleVar_Alpha, 0.5F};
        const ui::Group group;
        const ui::ItemWidth width{100.0F};
        const ui::Indent indent;
        const ui::Disabled disabled;
        if (const ui::Table table{"table", 2}) {
            EXPECT_EQ(Capture().colors, 1);
            return;
        }
        ADD_FAILURE() << "the table should have begun";
    };
    draw();
    EXPECT_EQ(Capture(), outside_);
}

TEST_F(ImGuiScopes, ExceptionsUnwindEveryScope) {
    const auto draw = [] {
        const ui::Window window{"Throwing"};
        const ui::Id id{"name"};
        const ui::StyleVar padding{ImGuiStyleVar_WindowPadding,
                                   ImVec2{2.0F, 2.0F}};
        const ui::Table table{"table", 3};
        throw std::runtime_error("drawing failed");
    };
    EXPECT_THROW(draw(), std::runtime_error);
    EXPECT_EQ(Capture(), outside_);
}
