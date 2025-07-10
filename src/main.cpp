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

#define INITIAL_WINDOW_WIDTH 800
#define INITIAL_WINDOW_HEIGHT 600

#define MAX_SETTINGS_WIDTH 200.0f
#define MIN_SETTINGS_WIDTH 160.0f
#define SETTINGS_HEIGHT 300.0f

struct AppState
{
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    float time_left_in_timer = 60.0;
    bool is_settings_opened = true;
    SDL_Texture* current_texture = nullptr;
    bool is_timer_running = false;
    ImVec2 canvas_size = {INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT};
    ImVec2 canvas_offset = {0.0f, 0.0f};
    std::string current_folder = "";
};

PathGrabBag grab_bag{};

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{

    AppState* new_app_state = new AppState;

    // Create Window
    if (!SDL_CreateWindowAndRenderer("Figure Drawing Tool", INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &new_app_state->window, &new_app_state->renderer))
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
    if (!filelist)
    {
        SDL_Log("Error Ocurred: %s", SDL_GetError());
        return;
    }
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
        if (entry.is_directory())
        {
            continue;
        }
        std::filesystem::path extension = entry.path().extension();
        if (extension != ".jpg" and extension != ".JPG") continue; // Lets support just JPG for now
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

void DrawCurrentTexture(AppState* state)
{
    if (state->current_texture != nullptr)
    {
        float img_w, img_h;
        SDL_GetTextureSize(state->current_texture, &img_w, &img_h);

        float aspect_ratio = state->canvas_size.y / img_h;

        SDL_FRect image_rect;
        image_rect.w = img_w * aspect_ratio;
        image_rect.h = img_h * aspect_ratio;

        float available_side_space = (state->canvas_size.x - image_rect.w) / 2;
        
        image_rect.x = available_side_space + state->canvas_offset.x;
        image_rect.y = state->canvas_offset.y;

        SDL_RenderTexture(state->renderer, state->current_texture, NULL, &image_rect);
    }
}

bool DrawSettingsWindow(AppState* state)
{
    if (state->is_settings_opened)
    {
        ImGui::Begin("Settings", &(state->is_settings_opened), ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
        if (state->current_folder.empty())
        {
            ImGui::Text("No Folder Selected");
        }
        else
        {
            ImGui::Text(state->current_folder.c_str());
        }
        ImVec2 text_rect_size = ImGui::GetItemRectSize();
        float settings_window_width = std::clamp(text_rect_size.x, MIN_SETTINGS_WIDTH, MAX_SETTINGS_WIDTH);
        ImGui::SetWindowSize({settings_window_width, SETTINGS_HEIGHT});
        
        bool button_clicked = ImGui::Button("Select Folder", {100.0, 20.0});
        if (button_clicked)
        {
            SDL_ShowOpenFolderDialog(DialogCallback, state, state->window, NULL, false);
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
        ImGui::End();
        return true;
    }
    return false;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    AppState* state = static_cast<AppState*>(appstate);

    int window_h, window_w;
    SDL_GetWindowSize(state->window, &window_w, &window_h);

    SDL_SetRenderDrawColorFloat(state->renderer, 0.0f, 0.0f, 0.0f, SDL_ALPHA_OPAQUE_FLOAT);
    SDL_RenderClear(state->renderer);



    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::BeginMainMenuBar();
    ImGui::EndMainMenuBar();

    ImVec2 main_menu_bar_rect = ImGui::GetItemRectSize();
    
    state->canvas_size.x = window_w;
    state->canvas_size.y = window_h - main_menu_bar_rect.y;

    state->canvas_offset.y = main_menu_bar_rect.y;
    
    DrawCurrentTexture(state);
    
    
    ImGui::SetNextWindowPos({0.0, main_menu_bar_rect.y});

    if (DrawSettingsWindow(state))
    {
        auto Window1Size = ImGui::GetWindowSize();
        ImGui::SetNextWindowPos({0.0, Window1Size.y + 16.0f});
    }
    else
    {
        ImGui::SetNextWindowPos({0.0, 0.0});
    }

    ImGui::Begin("Timer", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImGui::Text("Hello From Another thingy");
    ImGui::GetItemRectSize();
    ImGui::End();
    
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

