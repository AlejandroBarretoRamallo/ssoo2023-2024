#include <iostream>
#include <vector>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Wrong number of arguments\n";
    return EXIT_FAILURE;
  }
  std::string filename(argv[1]);
  std::vector<std::string> command_args;
  for (int i = 2; i < argc; ++i) {
    command_args.emplace_back(argv[i]);
  }
  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
  int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
  if (fd < 0) {
    std::cerr << "Error at open\n";
    return EXIT_FAILURE;
  }
  int fds[2];
  int return_code = pipe(fds); // fds[0] es el extremo de lectura, fds[1] el de escritura
  if (return_code < 0)  {
    std::cerr << "Error AT Pipe\n";
    close(fd);
    return EXIT_FAILURE;
  }
  pid_t pid = fork(); // crear un proceso hijo
  if (pid > 0) {
    close(fds[1]);
    // do 
    //nbytes = FUNCION READ-FILE PASANDO EL FDS[0]; BUFFER, 256
    //send to == write(fd, buffer, nbytes), aunque para mi seria el send to
    // }while nbytes > 0;
    //todo al reves en el modo recibir
    int status;
    //Parent
    bool flag = true;
    while (flag) {
      int res = waitpid(pid, &status, 0);
      if (res != 0) {
        flag = false;
      }      
    }
    
    close(fd);
    return EXIT_SUCCESS;
  }
  else if(pid == 0) {
    //Child
    close(fds[0]);
    dup2(fds[1], STDOUT_FILENO);
    std::vector<const char *> exec_args;
    for (auto &str : command_args) {
      exec_args.push_back(str.c_str());
    }
    //Add nullptr
    exec_args.push_back(nullptr);
    int exec_rsult = execvp(exec_args[0], const_cast<char * const *> (exec_args.data()));
    std::cerr << "Error at EXEC\n";
    return errno;
  }
  else {
    std::cerr << "Error at fork\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}