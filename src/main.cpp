#include <iostream>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h> // What a Include does is get the code in here XD So we are just adding the SDL main code into this file and creating all the functions here
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

struct AppState
{
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    bool is_settings_opened = true;
};

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{

    AppState* new_app_state = new AppState;

    // Create Window
    if (!SDL_CreateWindowAndRenderer("Figure Drawing Tool", 800, 600, 0, &new_app_state->window, &new_app_state->renderer))
    {
        SDL_Log("Coudn't Create Window or Renderer: %s", SDL_GetError());
        delete new_app_state;
        return SDL_APP_FAILURE;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplSDL3_InitForSDLRenderer(new_app_state->window, new_app_state->renderer);
    ImGui_ImplSDLRenderer3_Init(new_app_state->renderer);

    *appstate = new_app_state;

    return SDL_APP_CONTINUE;
}

void DialogCallback (void *userdata, const char * const *filelist, int filter)
{

}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    AppState* state = static_cast<AppState*>(appstate);

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    if (state->is_settings_opened)
    {
        ImGui::Begin("Settings", &(state->is_settings_opened), ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
        ImGui::SetWindowPos({0.0, 0.0});
        ImGui::Text("Welcome To ImGui");
        bool button_clicked = ImGui::Button("Button Here", {100.0, 100.0});
        if (button_clicked)
        {
            SDL_ShowOpenFolderDialog(DialogCallback, NULL, state->window, NULL, false);
        }
        auto Window1Size = ImGui::GetWindowSize();
        ImGui::SetNextWindowPos({0.0, Window1Size.y + 16.0f});
        ImGui::End();
    }
    else
    {
        ImGui::SetNextWindowPos({0.0, 0.0});
    }

    ImGui::Begin("Second UI", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImGui::Text("Hello From Another thingy");
    ImGui::End();
    
    ImGui::Render();
    
    SDL_SetRenderDrawColorFloat(state->renderer, 0.0f, 0.0f, 0.0f, SDL_ALPHA_OPAQUE_FLOAT);
    SDL_RenderClear(state->renderer);

    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), state->renderer);

    SDL_RenderPresent(state->renderer);

    return SDL_APP_CONTINUE;
}

SDL_AppResult ProcessKeyUp(void* appstate, SDL_Event* event)
{
    switch(event->key.key)
    {
        case SDLK_S:

        default:
            break;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    ImGui_ImplSDL3_ProcessEvent(event);
    switch (event->type)
    {
        case SDL_EVENT_KEY_UP:
            return ProcessKeyUp(appstate, event);
            break;
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
        default:
            break;
    }
    if (event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS;

    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    AppState* state = static_cast<AppState*>(appstate);

    SDL_DestroyRenderer(state->renderer);
    SDL_DestroyWindow(state->window);

    delete appstate;
}

