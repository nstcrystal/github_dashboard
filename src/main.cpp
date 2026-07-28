#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>


using namespace ftxui;


using json = nlohmann::json;


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


std::string getGithubUser(
    const std::string& username
)
{
    CURL* curl = curl_easy_init();

    std::string response;


    if(curl)
    {
        std::string url =
            "https://api.github.com/users/"
            + username;


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



int main()
{
    auto data =
        getGithubUser("NSTCrystal");


    auto jsonData =
        json::parse(data);


    std::cout
        << "Username: "
        << jsonData["login"]
        << "\n";


    std::cout
        << "Followers: "
        << jsonData["followers"]
        << "\n";


    std::cout
        << "Repos: "
        << jsonData["public_repos"]
        << "\n";
}