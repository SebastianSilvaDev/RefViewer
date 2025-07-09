#include <iostream>
#include <filesystem>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h> // What a Include does is get the code in here XD So we are just adding the SDL main code into this file and creating all the functions here

#include "SDL3_image/SDL_image.h"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include "PathGrabBag.h"

struct AppState
{
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    std::string current_folder = "";
    bool is_settings_opened = true;
    SDL_Texture* current_texture = nullptr;
};

PathGrabBag grab_bag{};

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

    SDL_SetWindowResizable(new_app_state->window, true);

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
    if (!*filelist)
    {
        SDL_Log("Failed To Get a Path");
        return;
    }
    if (userdata)
    {
       auto app_state = static_cast<AppState*>(userdata);
       app_state->current_folder = *filelist;
    }
    SDL_Log("File Selected %s", *filelist);
    std::string path(*filelist);
    auto it = std::filesystem::recursive_directory_iterator(path);
    grab_bag.ResetCollection();
    for (auto& entry : std::filesystem::recursive_directory_iterator(path))
    {
        SDL_Log("File Found: %s", entry.path().string().c_str());
        grab_bag.AddPathToGrabBag(entry.path());
    }
    grab_bag.InitializeBag();
}

void LoadNextImage(AppState* appstate)
{
    SDL_Texture* new_texture;

    auto texture_path = grab_bag.GetNext();

    new_texture = IMG_LoadTexture(appstate->renderer, texture_path->string().c_str());

    appstate->current_texture = new_texture;
}

void UpdateCurrentTexture(AppState* appstate)
{

}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    AppState* state = static_cast<AppState*>(appstate);

    SDL_SetRenderDrawColorFloat(state->renderer, 0.0f, 0.0f, 0.0f, SDL_ALPHA_OPAQUE_FLOAT);
    SDL_RenderClear(state->renderer);

    if (state->current_texture != nullptr)
    {
        float img_w, img_h;
        SDL_GetTextureSize(state->current_texture, &img_w, &img_h);

        int window_w, window_h;

        SDL_GetWindowSize(state->window, &window_w, &window_h);

        float aspect_ratio = (float)window_h / img_h;

        SDL_FRect image_rect;
        image_rect.w = img_w * aspect_ratio;
        image_rect.h = img_h * aspect_ratio;

        float available_side_space = ((float)window_w - image_rect.w) / 2;
        
        image_rect.x = available_side_space;
        image_rect.y = 0.0;

        SDL_RenderTexture(state->renderer, state->current_texture, NULL, &image_rect);
    }

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    if (state->is_settings_opened)
    {
        ImGui::Begin("Settings", &(state->is_settings_opened), ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
        ImGui::SetWindowSize({160.0, 300.0});
        ImGui::SetWindowPos({0.0, 0.0});
        if (state->current_folder.empty())
        {
            ImGui::Text("No Folder Selected");
        }
        else
        {
            ImGui::Text(state->current_folder.c_str());
        }
        bool button_clicked = ImGui::Button("Select Folder", {100.0, 20.0});
        if (button_clicked)
        {
            SDL_ShowOpenFolderDialog(DialogCallback, appstate, state->window, NULL, false);
        }
        if (grab_bag.IsGrabBagReady())
        {
            auto b_start_button_clicked = ImGui::Button("Start", {40.0, 20.0});
            ImGui::SameLine();
            if (b_start_button_clicked)
            {
                LoadNextImage(state);
            }
            ImGui::Button("Skip", {40.0, 20.0});
            ImGui::SameLine();
            ImGui::Button("Stop", {40.0, 20.0});
        }
        auto Window1Size = ImGui::GetWindowSize();
        ImGui::SetNextWindowPos({0.0, Window1Size.y + 16.0f});
        ImGui::End();
    }
    else
    {
        ImGui::SetNextWindowPos({0.0, 0.0});
    }

    // ImGui::Begin("Second UI", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    // ImGui::Text("Hello From Another thingy");
    // ImGui::End();
    
    ImGui::Render();
    
    

    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), state->renderer);

    SDL_RenderPresent(state->renderer);

    return SDL_APP_CONTINUE;
}

SDL_AppResult ProcessKeyUp(void* appstate, SDL_Event* event)
{
    AppState* state = static_cast<AppState*>(appstate);

    switch(event->key.key)
    {
        case SDLK_S:
            state->is_settings_opened = true;
            break;
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

    if (state->current_texture != nullptr)
    {
        SDL_DestroyTexture(state->current_texture);
    }

    SDL_DestroyRenderer(state->renderer);
    SDL_DestroyWindow(state->window);

    delete appstate;
}

