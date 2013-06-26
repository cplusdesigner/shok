#ifndef _Proc_h_
#define _Proc_h_

/* Parent-child process communication
 *
 * A Proc represents an interface between a parent and a child process.
 *
 * The parent constructs a Proc implementation and calls its .run(), which will
 * fork off a child.
 *
 * Proc is virtual -- an implementor represents the child process and provides
 * a child_exec() function, which is automatically called by the child, when
 * the parent calls Proc.run().  child_exec() is expected to exec a new
 * process, and thus never return.  The Proc and its members disappear to the
 * child.  The child's stdin and stdout redirect to communication channels with
 * the parent (unless pipechat is set false).  Its stderr remains connected to
 * the parent's stderr stream.
 *
 * The parent is left with the Proc, and communicates to the child via the
 * Proc.in and Proc.out member streams (unless pipechat is set false).
 */

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
namespace io = boost::iostreams;

#include <sys/wait.h>
#include <cstdio>
#include <fcntl.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <vector>
using std::string;
using std::vector;

class Proc {
public:
  Proc(const string& name)
    : name(name), cmd(name), pipechat(true) {}

  ~Proc() {
    finish();
  }

  void run() {
    const int READ = 0;
    const int WRITE = 1;

    int tochild[2];
    int toparent[2];
    if (pipechat) {
      makePipe(tochild, "to child");
      makePipe(toparent, "to parent");
    }

    pid = fork();
    if (pid == -1) {
      perror((name + " fork").c_str()); exit(1);
    }

    if (pid == 0) {
      // child
      child_init();

      if (pipechat) {
        closePipe(tochild[WRITE], "child to child write");
        closePipe(toparent[READ], "child to parent read");

        if (STDIN_FILENO != tochild[READ]) {
          dupPipe(tochild[READ], STDIN_FILENO, "child to child input");
          closePipe(tochild[READ], "child to child input after dup");
        }
        if (STDOUT_FILENO != toparent[WRITE]) {
          dupPipe(toparent[WRITE], STDOUT_FILENO, "child to parent output");
          closePipe(toparent[WRITE], "child to parent output after dup");
        }

        // set stdin and stdout line-buffered
        setlinebuf(stdout);
        setlinebuf(stdin);
      }

      child_exec();
      perror((name + " failed to exit").c_str());
      _exit(1);
    }

    // parent
    if (pipechat) {
      closePipe(tochild[READ], "parent to child read");
      closePipe(toparent[WRITE], "parent to parent write");
      in.open(const_cast<const int&>(toparent[READ]), io::close_handle);
      out.open(const_cast<const int&>(tochild[WRITE]), io::close_handle);

      // Close the in and out file descriptors on any subsequent exec
      closeOnExec(in->handle(), "parent to parent input");
      closeOnExec(out->handle(), "parent to child output");
    }
  }

  void finish() {
    if (in.is_open()) in.close();
    if (out.is_open()) out.close();
  }

  const string name;
  // Enables in/out pipes for communication between the parent and the child's
  // stdin/stdout.  Default: true.  If false, the child inherits the parent's
  // stdin/stdout (sketchy).
  bool pipechat;
  pid_t pid;
  string cmd;           // command for child to invoke; defaults to name
  vector<string> args;  // args for the command
  // Streams only used by parent
  io::stream<io::file_descriptor_source> in;
  io::stream<io::file_descriptor_sink> out;

protected:
  virtual void child_init() {}
  virtual void child_exec() {
    string path = "./";
    path += cmd;
    size_t lenargv = std::max((size_t)2, args.size()+1);
    char** argv = new char*[lenargv];
    argv[0] = const_cast<char*>(cmd.c_str());
    int i = 0;
    for (vector<string>::const_iterator it = args.begin();
         it != args.end(); ++it, ++i) {
      argv[i] = const_cast<char*>(it->c_str());
    }
    argv[lenargv-1] = NULL;
    char *const env[] = { NULL };
    execve(path.c_str(), argv, env);
    delete[] argv;
  }

  void makePipe(int fds[2], const std::string& msg) {
    if (pipe(fds) == -1) {
      perror((name + " pipe " + msg).c_str()); exit(1);
    }
  }

  void closePipe(int fd, const std::string& msg) {
    if (-1 == close(fd)) {
      perror((name + " failed to close pipe " + msg).c_str());
      _exit(1);
    }
  }

  void dupPipe(int oldfd, int newfd, const std::string& msg) {
    if (-1 == dup2(oldfd, newfd)) {
      perror((name + " failed to dup " + msg).c_str());
      _exit(1);
    }
  }

  void closeOnExec(int fd, const std::string& msg) {
    if (0 != fcntl(fd, F_SETFD, 1)) {
      perror((name + " failed to set close-on-exec flag for " + msg).c_str());
      _exit(1);
    }
  }
};

#endif // _Proc_h_
