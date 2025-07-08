#include "PathGrabBag.h"

#include <iterator>
#include <algorithm>
#include <random>
#include <ranges>

PathGrabBag::PathGrabBag()
{
    size_t amount_of_places_to_reserve = 5000;
    m_path_collection.reserve(amount_of_places_to_reserve);
    m_paths_used.reserve(amount_of_places_to_reserve);
}

void PathGrabBag::AddPathToGrabBag(const std::filesystem::path &in_path)
{
    m_path_collection.push_back(in_path);
    
}

void PathGrabBag::ResetCollection()
{
    m_path_collection.clear();
}

void PathGrabBag::InitializeBag()
{
    m_paths_to_grab = {};
    m_paths_used.clear();

    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(m_path_collection.begin(), m_path_collection.end(), g);

    for (auto& path : m_path_collection)
    {
        m_paths_to_grab.push(&path);
    }
}

std::filesystem::path *PathGrabBag::GetNext()
{
    if (auto& path = m_paths_to_grab.front())
    {
        m_paths_to_grab.pop();
        m_paths_used.push_back(path);

        return path;
    }

    return nullptr;
}

void PathGrabBag::RestockBag()
{
    if (m_paths_used.empty())
    {
        return;
    }
    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(m_paths_used.begin(), m_paths_used.end(), g);

    for (auto& path : m_paths_used)
    {
        m_paths_to_grab.push(path);
    }

    m_paths_used.clear();
}
