#include <httplib.h>
#include <kutils.hpp>
#include <string>
#include <map>
#include <logger.hpp>

using namespace kiq::log;
//------------------------------------------------
static std::map<std::string, std::string> g_paths{
  {"log", R"(/log/(\d+)/([a-zA-Z0-9._]+))"}
};
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
    svr_.Post(g_paths.at("log"), [this](const auto& req, auto& res) { handle_log(req, res); });
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
      res.status = 200;
      res.set_content(to_json_response(build_id, filename), "application/json");
    }
    else
    {
      res.status = 400;
      res.set_content(R"({"httpcode": 400, "error": "Bad request: data missing"})", "application/json");
    }
  }
  //-------------------------------------------------------------------
  httplib::Server svr_;
  int             port_;
};