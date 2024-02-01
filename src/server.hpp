#include <httplib.h>
#include <kutils.hpp>
#include <string>
#include <map>
#include <logger.hpp>

using namespace kiq::log;

//------------------------------------------------
static std::map<std::string, std::string> g_paths{
  {"log", R"(/log/(\w+)/([a-zA-Z0-9._-]+))"},
  {"view", R"(/view/(\w+))"}
};
//------------------------------------------------
static const char* g_log_ext = ".log";
//------------------------------------------------
std::string to_file_path(const std::string& build_id, const std::string& name)
{
  return build_id + std::string{"/"} + name;
}
//------------------------------------------------
std::string to_dir_name(const std::string& id)
{
  return std::string{"logs/"} + id;
}
//------------------------------------------------
std::string to_json_response(const std::string& id, const std::string& name)
{
  return R"({"status": "success", "build_id": ")" + id +
         R"(", "filename": ")"                    + name + R"("})";
}
//------------------------------------------------

class server
{
 public:
  server(int port)
  : port_(port)
  {
    svr_.set_mount_point("/logs", "logs");
    svr_.Post(g_paths.at("log") , [this](const auto& req, auto& res) { handle_log (req, res); });
    svr_.Get (g_paths.at("view"), [this](const auto& req, auto& res) { handle_view(req, res); });
  }
  //----------------------------
  void run()
  {
    klog().i("Listening on port {}", port_);
    svr_.listen("localhost", port_);
  }

 private:
  void handle_log(const httplib::Request& req, httplib::Response& res)
  {
    const std::string build_id = req.matches[1];
    const std::string filename = req.matches[2];
    const auto        data     = req.body;
    const std::string dir_name = to_dir_name(build_id);

    klog().d("POST request to create log resource for build {} with filename {}", build_id, filename);

    if (!data.empty() && kutils::create_dir(dir_name))
    {
      kutils::SaveToFile(data, to_file_path(dir_name, filename));
      klog().t("File saved");
      res.status = 200;
      res.set_content(to_json_response(build_id, filename), "application/json");
      return;
    }

    res.status = 400;
    res.set_content(R"({"httpcode": 400, "error": "Bad request: data missing"})", "application/json");
  }
  //-------------------------------------------------------------------
  void handle_view(const httplib::Request& req, httplib::Response& res)
  {
    namespace fs = std::filesystem;
    try
    {
      std::string build_id     = req.matches[1];
      std::string path = "logs/" + build_id;

      klog().i("Request to view build {} logs", build_id);

      std::vector<std::string> logs;
      for (const auto& entry : fs::directory_iterator(path))
        if (entry.is_regular_file() && entry.path().extension() == g_log_ext)
          logs.push_back(entry.path().string());

      klog().d("{} logs discovered", logs.size());

      std::ostringstream content;
      for (const auto& log : logs)
        content << "<div><h3>" << log << "</h3>" << "<a href=\"/"
                << log << "\">Download</a><pre>" << kutils::ReadFile(log) << "</pre></div>";

      std::ostringstream html;
      html << "<!DOCTYPE html>"
           << "<html>"
           << "<head>"
           << "<title>Build " << build_id << "</title>"
           << "<style>"
           << "body { background-color: #1e1e1e; color: #ffffff; }"
           << ".log-entry { background-color: #2e2e2e; margin: 10px; padding: 10px; border-radius: 5px; }"
           << "</style>"
           << "</head>"
           << "<body>"
           << "<h1 style=\"color: #61dafb;\">Logs for build " << build_id << "</h1>"
           << content.str()
           << "</body>"
           << "</html>";

      res.set_content(html.str(), "text/html");
    }
    catch (const std::exception& e)
    {
      std::cerr << "Error: " << e.what() << std::endl;
      res.status = 500;
      res.set_content("Internal Server Error", "text/plain");
    }
  }
  //-------------------------------------------------------------------
  httplib::Server svr_;
  int             port_;
};