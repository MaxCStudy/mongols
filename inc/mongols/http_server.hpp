#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP


#include <string>
#include <ctime>
#include <memory>
#include <utility>
#include <list>
#include "tcp_server.hpp"
#include "request.hpp"
#include "response.hpp"
#include "http_request_parser.hpp"
#include "lib/leveldb/db.h"
#include "lib/leveldb/options.h"
#include "lib/LRUCache11.hpp"



namespace mongols {

    class http_server {
    public:
        http_server() = delete;
        http_server(const std::string& host, int port
                , int timeout = 5000
                , size_t buffer_size = 8192
                , size_t thread_size = std::thread::hardware_concurrency()
                , size_t max_body_size = 4096
                , int max_event_size = 64);
        virtual~http_server();

    public:
        void run(const std::function<bool(const mongols::request&)>& req_filter
                , const std::function<void(const mongols::request&, mongols::response&)>& res_filter);

        void set_session_expires(long long);
        void set_cache_expires(long long);
        void set_enable_session(bool);
        void set_enable_cache(bool);
        void set_max_open_files(int);
        void set_write_buffer_size(size_t);
        void set_max_file_size(size_t);
        void set_cache_size(size_t);
        void set_enable_compression(bool);
        void set_db_path(const std::string&);
        void set_enable_lru_cache(bool);
        void set_lru_cache_expires(long long);
        void set_lru_cache_size(size_t);
        void set_uri_rewrite(const std::pair<std::string, std::string>&);
        bool set_openssl(const std::string&, const std::string&
                , openssl::version_t = openssl::version_t::TLSv12
                , const std::string& ciphers = openssl::ciphers
                , long flags = openssl::flags);
        void set_enable_blacklist(bool);
        void set_enable_security_check(bool);
    private:
        std::string work(
                const std::function<bool(const mongols::request&)>& req_filter
                , const std::function<void(const mongols::request& req, mongols::response&)>& res_filter
                , const std::pair<char*, size_t>&
                , bool&
                , bool&
                , tcp_server::client_t&
                , tcp_server::filter_handler_function&);
    private:

        enum zip_t {
            deflate,
            gzip,
            unkown
        };
        std::string create_response(mongols::response& res, bool b);
        std::string get_status_text(int status);
        std::string tolower(std::string&);
        void upload(mongols::request&, const std::string&);
        std::string serialize(const std::unordered_map<std::string, std::string>&);
        void deserialize(const std::string&, std::unordered_map<std::string, std::string>&);
        bool deflate_compress(std::string&);
        bool gzip_compress(std::string&);
    private:

        class cache_t {
        public:

            cache_t();
            ~cache_t() = default;
            int status;
            time_t t;
            std::string content_type, content;
            bool enable_zip;
            http_server::zip_t zip_type;

            bool expired(long long expires)const;
        };


    private:
        mongols::tcp_server *server;
        size_t max_body_size, lru_cache_size;
        leveldb::DB *db;
        leveldb::Options db_options;
        long long session_expires, cache_expires, lru_cache_expires;
        bool enable_session, enable_cache, enable_lru_cache;
        std::string db_path;
        std::vector<std::pair<std::string, std::string>> uri_rewrite_config;
        lru11::Cache<std::string, std::shared_ptr<cache_t>>*lru_cache;

    public:
        static int zip_level;
        static size_t zip_min_size, zip_max_size;
        static std::list<std::string> zip_mime_type;
    };


}

#endif /* HTTP_SERVER_HPP */

