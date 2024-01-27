#include <httplib.h>
#include <kutils.hpp>
#include <string>
#include <map>
#include <logger.hpp>

using namespace kiq::log;

static std::map<std::string, std::string> g_paths{
  {"log", R"(/log/(\d+)/([a-zA-Z0-9._]+))"}
};
//------------------------------------------------
// template <typename T = std::string>
std::string to_file_path(const std::string& build_id, const std::string& name)
{
  return build_id + std::string{"/"} + name;
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
    const auto build_id = req.matches[1];
    const auto filename = req.matches[2];
    const auto data     = req.body;
    if (!data.empty() && kutils::create_dir(build_id))
    {
      kutils::SaveToFile(data, to_file_path(build_id, filename));
      res.status = 200;
      res.set_content("Log received and stored.", "text/plain");
    }
    else
    {
      res.status = 400;
      res.set_content("Bad Request: data missing.", "text/plain");
    }
  }
  //-------------------------------------------------------------------
  httplib::Server svr_;
  int             port_;
};