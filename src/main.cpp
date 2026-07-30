#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <algorithm>


using namespace ftxui;


using json = nlohmann::json;


// Cấu trúc repo
struct Repo
{
    std::string name;
    int stars;
    std::string language;
};



size_t WriteCallback(
    void* contents,
    size_t size,
    size_t nmemb,
    std::string* output
)
{
    size_t totalSize = size * nmemb;
    output->append(
        (char*)contents,
        totalSize
    );

    return totalSize;
}


std::string githubRequest(
    const std::string& username
)
{
    CURL* curl = curl_easy_init();

    std::string response;


    if(curl)
    {
        std::string url = "https://api.github.com/users/" + username;


        curl_easy_setopt(
            curl,
            CURLOPT_URL,
            url.c_str()
        );


        curl_easy_setopt(
            curl,
            CURLOPT_WRITEFUNCTION,
            WriteCallback
        );


        curl_easy_setopt(
            curl,
            CURLOPT_WRITEDATA,
            &response
        );


        struct curl_slist* headers = nullptr;

        headers = curl_slist_append(
            headers,
            "User-Agent: crystal-dashboard"
        );


        curl_easy_setopt(
            curl,
            CURLOPT_HTTPHEADER,
            headers
        );


        curl_easy_perform(curl);


        curl_easy_cleanup(curl);
    }


    return response;
}


// Hàm lấy repository.
std::vector<Repo> getRepos(const std::string& username)
{
    auto response =
        githubRequest(username + "/repos");

    auto reposJson =
        json::parse(response);

    std::vector<Repo> repos;

    for (auto& repo : reposJson)
    {
        repos.push_back(
            Repo{
                repo["name"].get<std::string>(),
                repo["stargazers_count"].get<int>(),
                repo["language"].is_null()
                    ? "Unknown"
                    : repo["language"].get<std::string>()
            }
        );
    }

    return repos;
}


int main()
{
    const std::string username = "nstcrystal";

    json userData;
    std::vector<Repo> repos;

    int totalStars = 0;
    int cpp = 0;
    int python = 0;
    int ts = 0;

    auto refresh = [&]()
    {
        userData = json::parse(githubRequest(username));

        repos = getRepos(username);

        std::sort(
            repos.begin(),
            repos.end(),
            [](const Repo& a, const Repo& b)
            {
                return a.stars > b.stars;
            });

        totalStars = 0;
        cpp = 0;
        python = 0;
        ts = 0;

        for (const auto& repo : repos)
        {
            totalStars += repo.stars;

            if (repo.language == "C++")
                cpp++;
            else if (repo.language == "Python")
                python++;
            else if (repo.language == "TypeScript")
                ts++;
        }
    };

    refresh();

    auto screen = ScreenInteractive::TerminalOutput();

    auto dashboard = CatchEvent(

        Renderer([&]
        {
            Elements left;
            Elements right;

            // ===== Left Panel =====
            left.push_back(
                text("Profile")
                | bold
                | color(Color::Yellow));

            left.push_back(separator());

            left.push_back(
                text("Username: " +
                userData["login"].get<std::string>()));

            left.push_back(
                text("Followers: " +
                std::to_string(
                    userData["followers"].get<int>())));

            left.push_back(
                text("Public Repos: " +
                std::to_string(
                    userData["public_repos"].get<int>())));

            left.push_back(
                text("Total Stars: " +
                std::to_string(totalStars)));

            left.push_back(separator());

            left.push_back(
                text("Languages")
                | bold
                | color(Color::Green));

            left.push_back(
                text("C++        : " + std::to_string(cpp)));

            left.push_back(
                text("Python     : " + std::to_string(python)));

            left.push_back(
                text("TypeScript : " + std::to_string(ts))
            );

            // ===== Right Panel =====
            right.push_back(
                text("Top Repositories")
                | bold
                | color(Color::Cyan));

            right.push_back(separator());

            for (size_t i = 0;
                 i < std::min(repos.size(), size_t(10));
                 ++i)
            {
                const auto& repo = repos[i];

                right.push_back(
                    vbox({
                        text("★ " + repo.name) | bold,
                        text(
                            "  " +
                            repo.language +
                            " • ⭐" +
                            std::to_string(repo.stars))
                    }));
            }

            return vbox({

                text("Crystal Dashboard")
                | bold
                | center
                | color(Color::Green),

                separator(),

                hbox({

                    vbox(left)
                    | border
                    | flex,

                    vbox(right)
                    | border
                    | flex

                }),

                separator(),

                text("[R] Refresh    [Q] Quit")
                | center
                | dim

            })
            | border
            | size(WIDTH, EQUAL, 90);

        }),

        [&](Event event)
        {
            if (event == Event::Character('r'))
            {
                refresh();
                return true;
            }

            if (event == Event::Character('q'))
            {
                screen.ExitLoopClosure()();
                return true;
            }

            return false;
        }

    );

    screen.Loop(dashboard);

    return 0;
}