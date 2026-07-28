#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>


using namespace ftxui;


using json = nlohmann::json;


// Cấu trúc repo
struct Repo {
    std::string name;
    int stars;
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
                repo["stargazers_count"].get<int>()
            }
        );
    }

    return repos;
}


int main()
{
    const std::string username = "nstcrystal";

    // Lấy thông tin user
    auto userData = json::parse(githubRequest(username));

    // Lấy danh sách repository
    auto repos = getRepos(username);

    auto screen = ScreenInteractive::TerminalOutput();

    auto dashboard = Renderer([&] {

        Elements elements;

        elements.push_back(
            text("Crystal Dashboard")
            | bold
            | color(Color::Green)
            | center
        );

        elements.push_back(separator());

        elements.push_back(
            text("Username: " +
                userData["login"].get<std::string>())
        );

        elements.push_back(
            text("Followers: " +
                std::to_string(userData["followers"].get<int>()))
        );

        elements.push_back(
            text("Public Repos: " +
                std::to_string(userData["public_repos"].get<int>()))
        );

        elements.push_back(separator());

        elements.push_back(
            text("Repositories")
            | bold
            | color(Color::Cyan)
        );

        // Hiển thị tối đa 10repo đầu
        for (size_t i = 0;
            i < std::min(repos.size(), size_t(10));
            ++i)
        {
            elements.push_back(
                // text("• " + repos[i].name)

                text(
                    "★ "
                    + repos[i].name
                    + " ("
                    + std::to_string(repos[i].stars)
                    + ")"
                )
                | color(Color::Yellow)
            );
        }

        return vbox(elements)
            | border
            | size(WIDTH, EQUAL, 50);
    });

    screen.Loop(dashboard);

    return 0;
}