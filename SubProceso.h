class subprocess
{
public:

enum class stdio{
in,
out,
err,
outerr
};

subprocess(const std::vector<std::string>& args, subprocess::stdio redirected_io);
private:
  bool is_alive();
  std::error_code exec(); 
  std::error_code wait();
  std::error_code kill();
  pid_t pid();
  int stdin_fd();
  int stdout_fd();
};