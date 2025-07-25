#include "BDMan.h"
#include <iostream>
#include <iomanip>
#include <curl/curl.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <filesystem>
#include <algorithm>
#include <indicators/block_progress_bar.hpp>
#include <indicators/progress_bar.hpp>
#include <indicators/cursor_control.hpp>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
using namespace std;
using namespace indicators;
namespace fs = std::filesystem;

string header_filename;
vector<thread> download_threads;

size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
    return fwrite(ptr, size, nmemb, stream);
}

curl_off_t get_file_size(CURL* curl, const string& url)
{
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    CURLcode result = curl_easy_perform(curl);
    if (result != CURLE_OK)
    {
        cerr << "FAILURE: failed to obtain file size." << curl_easy_strerror(result) << endl;
        return -1;
    }

    double cl;
    result = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &cl);

    if (result == CURLE_OK && cl > 0.0)
    {
        return static_cast<curl_off_t>(cl);
    }
}



ProgressBar Download_Bar
{
    option::BarWidth{80},
    option::Start{"{"},
    option::End{"}"},
    option::Fill{"="},
    option::Lead{">"},
    option::ForegroundColor{Color::green},
    option::ShowPercentage{true},
    option::ShowElapsedTime{true},
    option::ShowRemainingTime{true},
    option::PrefixText{"Downloading "},
    option::FontStyles{vector<FontStyle>{FontStyle::bold}},
};

int dl_progress_callback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    if (dltotal <= 0)
        return 0;

    float progress = (static_cast<float>(dlnow) / dltotal) * 100;


    Download_Bar.set_progress(progress);
    this_thread::sleep_for(chrono::milliseconds(100));


    show_console_cursor(true);
    /*if (dltotal < 1024) // bytes
    {
        cout << "\rDownloading: " << fixed << setprecision(2) << progress
            << "% (" << dlnow << " / " << dltotal << " B)         " << flush;
    }
    else if (dltotal < 1048576) // <1 MB => KB
    {
        double dltotal_kb = dltotal / 1e3;
        double dlnow_kb = dlnow / 1e3;
        cout << "\rDownloading: " << fixed << setprecision(2) << progress
            << "% (" << dlnow_kb << " / " << dltotal_kb << " KB)         " << flush;
    }
    else if (dltotal < 1073741824) // <1 GB => MB
    {
        double dltotal_mb = dltotal / 1e6;
        double dlnow_mb = dlnow / 1e6;
        cout << "\rDownloading: " << fixed << setprecision(2) << progress
            << "% (" << dlnow_mb << " / " << dltotal_mb << " MB)         " << flush;
    }
    else // GB
    {
        double dltotal_gb = dltotal / 1e9;
        double dlnow_gb = dlnow / 1e9;
        cout << "\rDownloading: " << fixed << setprecision(2) << progress
            << "% (" << dlnow_gb << " / " << dltotal_gb << " GB)         " << flush;
    }*/

    return 0;
}

string identify_filename(const string& download_url)
{
    size_t pos = download_url.find_last_of('/');
    if (pos != string::npos && pos + 1 < download_url.length())
    {
        return download_url.substr(pos + 1);
    }
    else
    {
        return "file.tmp";
    }
}

string quotes_remover(const string& str)
{
    if (str.length() >= 2 && str.front() == '"' && str.back() == '"')
    {
        return str.substr(1, str.length() - 2);
    }
    else
    {
        return str;
    }
}

size_t header_callback(char* buffer, size_t size, size_t nitems, void* userdata)
{
    size_t total_size = size * nitems;
    string header_line(buffer, total_size);

    string lower_line = header_line;
    std::transform(lower_line.begin(), lower_line.end(), lower_line.begin(),
        [](unsigned char c) { return std::tolower(c); });

    if (lower_line.find("content-disposition:") != string::npos)
    {
        size_t pos = lower_line.find("filename=");
        if (pos != string::npos)
        {
            size_t start = header_line.find('"', pos);
            size_t end = header_line.find('"', start + 1);
            if (start != string::npos && end != string::npos)
            {
                header_filename = header_line.substr(start + 1, end - start - 1);
                cout << "The detected filename is " << header_filename << "\n";
            }
        }
    }
    return total_size;
}

int main()
{

    string download_directory = "%USERPROFILE%/Downloads";
    string download_url;
    cout << "welcome to BDMan!\n";
    cout << "Set the target download file url : ";
    cin >> download_url;

    /*cout << "Enter the target download url : ";
    cin >> download_url;*/
    string filename = identify_filename(download_url);
    cout << "Initial file name from provided url : " << filename << "\n";

    
    //This part is still not complete:
    //Multithreaded download:
    /*curl_off_t file_size = get_file_size(nullptr, download_url);
    if (file_size > 0)
    {
        int thread_number = 8;
        long long segment_size = file_size / thread_number;

        for (int i = 0; i < thread_number; i++)
        {
            long long start = i * segment_size;
            long long end = (i == thread_number - 1) ? file_size - 1 : start + segment_size - 1;

            download_threads.emplace_back([start, end, download_url, i, thread_number]
            {
                CURL* curl_thread = curl_easy_init();
                string file_name = "file_" + to_string(i);
                FILE* fp = fopen(file_name.c_str(), "wb");
                if (curl_thread)
                {
                    string range = (to_string(start) + "-" + to_string(end));

                    curl_easy_setopt(curl_thread, CURLOPT_URL, download_url.c_str());
                    curl_easy_setopt(curl_thread, CURLOPT_WRITEFUNCTION, write_data);
                    curl_easy_setopt(curl_thread, CURLOPT_FOLLOWLOCATION, 1L);
                    curl_easy_setopt(curl_thread, CURLOPT_HEADERFUNCTION, header_callback);
                    curl_easy_setopt(curl_thread, CURLOPT_WRITEDATA, (fp + thread_number));
                    curl_easy_setopt(curl_thread, CURLOPT_XFERINFOFUNCTION, dl_progress_callback);
                    curl_easy_setopt(curl_thread, CURLOPT_NOPROGRESS, 0L);
                    curl_easy_setopt(curl_thread, CURLOPT_RANGE, range.c_str());

                    show_console_cursor(false);
                    CURLcode result = curl_easy_perform(curl_thread);
                    show_console_cursor(true);

                    if (result != CURLE_OK)
                    {
                        cerr << "FALURE: Thread " << i << " failed : " << curl_easy_strerror(result) << endl;
                    }
                    fclose(fp);
                    curl_easy_cleanup(curl_thread);
                }
            });
        }
    }*/
    

   
    CURL* curl = curl_easy_init();
    if (curl) {
        FILE* fp = nullptr;
        errno_t err = fopen_s(&fp, (download_directory + filename).c_str(), "wb");
        if (err != 0 || fp == nullptr) {
            cerr << "Failed to open file for writing.\n";
            curl_easy_cleanup(curl);
            return 1;
        }


        curl_easy_setopt(curl, CURLOPT_URL, download_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, dl_progress_callback);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);



        //download:
        show_console_cursor(false);
        CURLcode result = curl_easy_perform(curl);
        show_console_cursor(true);

        cout << endl;

        if (result == CURLE_OK)
        {
            Download_Bar.mark_as_completed();
            if (!header_filename.empty())
            {
                if (header_filename != filename)
                {
                    cout << "Renaiming " << filename << " to " << header_filename << "\n";
                    if (rename((download_directory + "/" + filename).c_str(), (download_directory + "/" + header_filename).c_str()) != 0)
                    {
                        cerr << "Error while renaming file.\n";
                    }

                }
            }
            cout << "File downloaded sucssesfully!\n";
        }

        if (result != CURLE_OK)
        {
            cerr << "FALURE: failed to download file : " << curl_easy_strerror(result) << endl;
        }

        fclose(fp);
        curl_easy_cleanup(curl);
    }
    
    
    else {
        cerr << "Failed to initialize CURL.\n";
    }
    return 0;
}