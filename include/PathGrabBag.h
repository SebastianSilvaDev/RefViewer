#pragma once

#include <filesystem>
#include <vector>
#include <queue>

class PathGrabBag
{

public:
    PathGrabBag();

    void AddPathToGrabBag(const std::filesystem::path& in_path);

    void ResetCollection();

    void InitializeBag();

    std::filesystem::path* GetNext();

    void RestockBag();

    inline bool IsGrabBagReady() const
    {
        return !m_path_collection.empty();
    }

private:
    std::vector<std::filesystem::path> m_path_collection;

    std::queue<std::filesystem::path*> m_paths_to_grab;

    std::vector<std::filesystem::path*> m_paths_used;


};