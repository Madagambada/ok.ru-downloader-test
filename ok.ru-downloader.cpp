#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <fstream>
#include <string> 
#include <future>
#include <utility>
#include <filesystem>

#include <curl/curl.h>

#include "strutil.h"
#include "httplib.h"
#include "cxxopts.hpp"

std::string head = "ok downloader 1.0.2 -final-\n";
bool stop = false;
std::vector<std::string> uidList, downloadListvid, downloadListuid, errorList, notfoundList;
std::vector <std::pair <std::string, std::string>> toDownload;
std::mutex mtx;
std::string folder;
std::string userAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/112.0.0.0 Safari/537.36";

httplib::Server svr;


static size_t curl_write_func(void* buffer, size_t size, size_t nmemb, void* param) {
    std::string& text = *static_cast<std::string*>(param);
    size_t totalsize = size * nmemb;
    text.append(static_cast<char*>(buffer), totalsize);
    return totalsize;
}

std::string curl_get(std::string uid) {
    CURL* curl;
    CURLcode res;
    std::string data;
    std::string url = "https://ok.ru/profile/" + uid + "/video";
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
        curl_easy_setopt(curl, CURLOPT_CAINFO, "/etc/ssl/certs/ca-certificates.crt");
        curl_easy_setopt(curl, CURLOPT_DNS_SERVERS, "1.1.1.1,1.0.0.1");
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/112.0.0.0 Safari/537.36");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_func);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::stringstream sstime;
            std::time_t etimr = std::time(nullptr);
            sstime << std::put_time(std::localtime(&etimr), "[%H:%M:%S, %d-%m-%Y] curl_get error ") << res << ": " << curl_easy_strerror(res) << ", at: " << url;
            std::cout << sstime.str();
            errorList.push_back(sstime.str());
            curl_easy_cleanup(curl);
            return std::string();
        }
        if (strutil::contains(data, "video-card_live __active")) {
            data = strutil::split(data, "video-card_live __active")[0];
            data = strutil::split(data, "video-card_img-w")[1];
            data = strutil::split(data, "href=\"")[1];
            data = strutil::split(data, "\"")[0];
            curl_easy_cleanup(curl);
            return data;
        }
        if (strutil::contains(data, "page-not-found")) {
            curl_easy_cleanup(curl);
            return "404";
        }

        curl_easy_cleanup(curl);
        return std::string();
    }
    curl_easy_cleanup(curl);
    return std::string();
}

std::string curl_get_test(std::string uid) {
    CURL* curl;
    CURLcode res;
    std::string data;
    std::string url = "https://example.com";
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
        curl_easy_setopt(curl, CURLOPT_CAINFO, "/etc/ssl/certs/ca-certificates.crt");
        curl_easy_setopt(curl, CURLOPT_DNS_SERVERS, "1.1.1.1,1.0.0.1");
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/112.0.0.0 Safari/537.36");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_func);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        return "";
    }
    curl_easy_cleanup(curl);
    return "";
}

bool vContains(std::vector<std::string> v, std::string s) {
    if (std::find(v.begin(), v.end(), s) == v.end()) {
        return 0;
    }
    return 1;
}

void renamefolder(std::string uid) {
    while (std::filesystem::exists(folder + "/vid-ok/" + uid)) {
        std::error_code ec;
        std::filesystem::rename((folder + "/vid-ok/" + uid), (folder + "/vid-ok/" + uid + "-404-"), ec);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void renamefolderR(std::string uid) {
    while (std::filesystem::exists(folder + "/vid-ok/" + uid)) {
        std::error_code ec;
        std::filesystem::rename((folder + "/vid-ok/" + uid), (folder + "/vid-ok/" + uid + "-r-"), ec);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void downloader(std::string vurl, std::string uid) {
    time_t t = std::time(nullptr);
    tm tm = *std::localtime(&t);
    std::stringstream sstime;
    sstime << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S_live_");
    std::string saveLocation = folder + "/vid-ok/" + uid + "/" + sstime.str() + uid + ".mkv";
    std::string command = "yt-dlp --downloader ffmpeg -o \"" + saveLocation + "\" -q https://ok.ru" + vurl;
    std::cout << "command:\n" + command << std::endl;
    system(command.c_str());

    mtx.lock();
    for (int i = 0; i < downloadListvid.size(); i++) {
        if (downloadListvid[i] == vurl) {
            downloadListvid.erase(downloadListvid.begin() + i);
            break;
        }
    }
    for (int i = 0; i < downloadListuid.size(); i++) {
        if (downloadListuid[i] == uid) {
            downloadListuid.erase(downloadListuid.begin() + i);
            break;
        }
    }
    mtx.unlock();
}

void writeToFile() {
    std::ofstream file(folder + "/ok-user.list");
    for (unsigned int i = 0; i < uidList.size(); i++) {
        file << uidList[i];
        if (!(i == uidList.size() - 1)) {
            file << "\n";
        }
    }
    file.close();
}

void readFile() {
    std::cout << "Read ok-user.list\n";
    std::ifstream file(folder + "/ok-user.list");
    std::string str;
    while (std::getline(file, str)) {
        uidList.push_back(str);
    }
    file.close();
}

void httpServer() {
    svr.Post("/", [](const httplib::Request& req, httplib::Response& res) {
        std::string msg = req.body;
        if (msg == "h") {
            res.body = (head +
                "Commands:\n" +
                "h - Help \n" +
                "d - List active downloads \n" +
                "a + User-ID - Add User-ID to database e.g. \"a 123456789\" \n" +
                "l - List database \n" + "r + User-ID - remove User-ID from database e.g. \"r 123456789\" \n" +
                "xa - Exit \n" + "e - list last 10 errors \n" +
                "ea - list all errors\n" + "v - list version\n" +
                "c + User-ID - check if User-ID is in database\n"
                );
        }
        else if (msg == "v") {
            res.body = (head + "Build with: \n\n" + std::string(curl_version()) + "\nhttplib/" + CPPHTTPLIB_VERSION + "\n");
        }
        else if (msg == "d") {
            mtx.lock();
            if (downloadListuid.size() > 0) {
                std::string tmp;
                for (unsigned int i = 0; i < downloadListuid.size(); i++) {
                    tmp.append(downloadListuid[i] + "\n");
                }
                mtx.unlock();
                res.body = (head + "Currently live:\n" + tmp);
            }
            else {
                mtx.unlock();
                res.body = (head + "Currently live:\n" + "None\n");
            }
        }
        else if (msg.find("a ") != std::string::npos) {
            if (msg.size() > 2) {
                msg.erase(0, 2);
                mtx.lock();
                if (std::find(uidList.begin(), uidList.end(), msg) == uidList.end()) {
                    mtx.unlock();
                    std::string vurl = curl_get(msg);
                    if (vurl == "404") {
                        res.body = (head + "\"" + msg + "\"" + " not found\n");
                    }
                    else if (!vurl.empty()) {
                        mtx.lock();
                        uidList.push_back(msg);
                        downloadListvid.push_back(vurl);
                        downloadListuid.push_back(msg);
                        mtx.unlock();
                        std::thread(downloader, vurl, msg).detach();
                        writeToFile();
                        res.body = (head + "\"" + msg + "\"" + " added + downloading\n");
                    }
                    else {
                        mtx.lock();
                        uidList.push_back(msg);
                        mtx.unlock();
                        writeToFile();
                        res.body = (head + "\"" + msg + "\"" + " added\n");
                    }
                }
                else {
                    mtx.unlock();
                    res.body = (head + "\"" + msg + "\"" + " already in the database\n");
                }
            }
        }
        else if (msg == "l") {
            std::string tmp = head + "In database (" + folder + "/ok-user.list" + "):\n";
            mtx.lock();
            for (unsigned int i = 0; i < uidList.size(); i++) {
                tmp.append(uidList[i] + "\n");
            }
            mtx.unlock();
            res.body = (tmp);
        }
        else if (msg.find("r ") != std::string::npos) {
            if (msg.size() > 2) {
                msg.erase(0, 2);
                mtx.lock();
                if (std::find(uidList.begin(), uidList.end(), msg) != uidList.end()) {
                    for (int i = 0; i < uidList.size(); i++) {
                        if (uidList[i] == msg) {
                            uidList.erase(uidList.begin() + i);
                            break;
                        }
                    }
                    mtx.unlock();
                    res.body = (head + "\"" + msg + "\"" + " deleted\n");
                }
                else {
                    mtx.unlock();
                    res.body = (head + "\"" + msg + "\"" + " not in database\n");
                }
            }
        }
        else if (msg == "xa") {
            stop = true;
            res.body = (head + "exit programm\n");
            svr.stop();
        }
        else if (msg == "ea") {
            std::stringstream tmpss;
            if (errorList.empty()) {
                tmpss << "No errors\n";
            }
            else {
                for (unsigned int i = 0; i < errorList.size(); i++) {
                    tmpss << errorList[i] + "\n";
                }
            }
            res.body = (head + tmpss.str());
        }
        else if (msg == "e") {
            std::stringstream tmpss;
            if (errorList.size() > 10) {
                for (unsigned int i = errorList.size() - 10; i < errorList.size(); i++) {
                    tmpss << errorList[i] + "\n";
                }
            }
            else if (errorList.empty()) {
                tmpss << "No errors\n";
            }
            else {
                for (unsigned int i = 0; i < errorList.size(); i++) {
                    tmpss << errorList[i] + "\n";
                }
            }
            res.body = (head + tmpss.str());
        }
        else if (msg.find("c ") != std::string::npos) {
            if (msg.size() > 2) {
                msg.erase(0, 2);
                mtx.lock();
                if (std::find(uidList.begin(), uidList.end(), msg) == uidList.end()) {
                    mtx.unlock();
                    res.body = (head + "\"" + msg + "\"" + " not in the database\n");
                }
                else {
                    mtx.unlock();
                    res.body = (head + "\"" + msg + "\"" + " in the database\n");
                }
            }
        }
        else {
            res.body = (head + "\"" + msg + "\" is not a valid command\n" );
        }
    });
    svr.listen("0.0.0.0", 34568);
}

void dworker() {
    std::vector <std::pair <std::string, std::string>> _toDownload;
    std::vector <std::string> _notfoundList;
    while (!stop) {
        mtx.lock();
        _toDownload = toDownload;
        _notfoundList = notfoundList;
        toDownload.clear();
        notfoundList.clear();
        mtx.unlock();

        if (!_toDownload.empty()) {
            for (int i = 0; i < _toDownload.size(); i++) {
                std::thread(downloader, _toDownload[i].first, _toDownload[i].second).detach();
            }
            _toDownload.clear();
        }

        if (!_notfoundList.empty()) {
            for (int i = 0; i < _notfoundList.size(); i++) {
                std::thread(renamefolder, _notfoundList[i]).detach();
            }
            _notfoundList.clear();
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void worker(std::vector<std::string> tuidList) {
    std::string vurl;
    for (int i = 0; i < tuidList.size(); i++) {
        vurl = curl_get_test(tuidList[i]);
        if (vurl.empty() || vurl == "404" || vContains(downloadListvid, vurl)) {
            if (vurl == "404") {
                mtx.lock();
                notfoundList.push_back(tuidList[i]);
                for (int o = 0; o < uidList.size(); o++) {
                    if (uidList[o] == tuidList[i]) {
                        uidList.erase(uidList.begin() + o);
                        break;
                    }
                }
                writeToFile();
                mtx.unlock();
            }
            continue;
        }
        mtx.lock();
        downloadListvid.push_back(vurl);
        downloadListuid.push_back(tuidList[i]);
        toDownload.push_back(std::make_pair(vurl, tuidList[i]));
        mtx.unlock();
        vurl.clear();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main(int argc, char* argv[]) {
    std::cout << head << std::endl;
    curl_global_init(CURL_GLOBAL_ALL);
    cxxopts::Options options(head);

    options.add_options()
        ("d,dir", "folder for save stuff", cxxopts::value<std::string>())
        ("h,help", "Print usage")
        ("v,version", "Print version")
        ;
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (result.count("version")) {
        std::cout << head + "Build with: \n\n" + std::string(curl_version()) + "\nhttplib/" + CPPHTTPLIB_VERSION << std::endl;
        return 0;
    }

    folder = result["d"].as<std::string>();

    readFile();
    std::thread(httpServer).detach();
    std::thread(dworker).detach();

    int workers = 4;

    while (!stop) {
        mtx.lock();
        std::vector<std::string> _uidList = uidList;
        mtx.unlock();

        int size = (_uidList.size() - 1) / workers + 1;

        std::vector< std::vector<std::string>> workerUidList;
        std::vector<std::string> _t;

        for (int i = 0; i < workers - 1; i++) {
            for (int k = 0; k < size; k++) {
                _t.push_back(_uidList[_uidList.size() - 1]);
                _uidList.pop_back();
            }
            workerUidList.push_back(_t);
            _t.clear();
        }
        workerUidList.push_back(_uidList);

        std::vector<std::future<void>> workerJobs;

        for (int i = 0; i < workers; i++) {
            workerJobs.push_back(std::async(worker, workerUidList[i]));
        }

        for (int i = 0; i < workerJobs.size(); i++) {
            workerJobs[i].get();
        }

        workerJobs.clear();
        workerUidList.clear();
    }

    curl_global_cleanup();
    return 0;
}
