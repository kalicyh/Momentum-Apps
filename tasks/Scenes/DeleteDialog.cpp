#include "DeleteDialog.hpp"

void FTasks::DeleteDialog::callback(const DialogExResult result, void* context) noexcept
{
    const auto* app = static_cast<UFZ::Application*>(context);
    if (result == DialogExResultRight)
    {
        const auto* ctx = CTX(app->getUserPointer());
        ctx->currentContainer->erase(ctx->currentContainer->begin() + static_cast<NoteContainer::difference_type>(ctx->currentNoteIndex));
    }
    SEND_CUSTOM_EVENT(app, Scenes::MAIN_MENU);
}

void FTasks::DeleteDialog::enter(void* context) noexcept
{
    auto* popup = GET_WIDGET_P(context, UFZ::DialogEx, Scenes::DELETE);
    auto* ctx = CTX(popup->application->getUserPointer());

    ctx->tmpBuffer = std::string(TASKS_UI_TEXT("Do you want to delete the following note: \"", "是否删除以下笔记: \""));
    ctx->tmpBuffer += (*ctx->currentContainer)[ctx->currentNoteIndex].first.c_str(); // Call c_str() because appending doesn't work I guess??
    ctx->tmpBuffer += std::string(TASKS_UI_TEXT("\"?", "\"?"));

    popup->reset();
    popup->setContext(popup->application)
            .setHeader(TASKS_UI_TEXT("Delete note?", "删除笔记?"), 64, 4, AlignCenter, AlignTop)
            .setIcon(-1, -1, nullptr)
            .setText(ctx->tmpBuffer.c_str(), 4, 16, AlignLeft, AlignTop)
            .setLeftButtonText(TASKS_UI_TEXT("No", "否"))
            .setCenterButtonText(nullptr)
            .setRightButtonText(TASKS_UI_TEXT("Yes", "是"))
            .setResultCallback(callback);
    RENDER_VIEW(popup->application, Scenes::DELETE);
}

bool FTasks::DeleteDialog::event(void* context, const SceneManagerEvent event) noexcept
{
    if (event.type == SceneManagerEventTypeCustom)
    {
        FORCE_NEXT_SCENE(static_cast<UFZ::Application*>(context), event.event);
        return true;
    }
    return false;
}

void FTasks::DeleteDialog::exit(void* context)
{
    GET_WIDGET_P(context, UFZ::DialogEx, Scenes::DELETE)->reset();
}