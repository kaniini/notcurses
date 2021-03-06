#define NCPP_EXCEPTIONS_PLEASE
#include <cstdlib>
#include <fcntl.h>
#include <getopt.h>
#include <iostream>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <filesystem>
#include <sys/types.h>
#include <ncpp/Direct.hh>
#ifndef __linux__
#define AT_NO_AUTOMOUNT 0 // not defined on freebsd
#endif

static void
usage(std::ostream& os, const char* name, int code){
  os << "usage: " << name << " -h | [ -lLR ] [ --align type ] paths...\n";
  os << " -d: list directories themselves, not their contents\n";
  os << " -l: use a long listing format\n";
  os << " -L: dereference symlink arguments\n";
  os << " -R: list subdirectories recursively\n";
  os << " -a|--align type: one of left, right, or center\n";
  os << " -h: print usage information\n";
  os << std::flush;
  exit(code);
}

// context as configured on the command line
struct lsContext {
  ncpp::Direct nc;
  bool longlisting;
  bool recursedirs;
  bool directories;
  bool dereflinks;
  ncalign_e alignment;
};

static int
handle_path(int dirfd, std::filesystem::path& dir, const char* p, const lsContext& ctx, bool toplevel);

// handle a single inode of arbitrary type
static int
handle_inode(std::filesystem::path& dir, const char* p, const struct stat* st, const lsContext& ctx){
  (void)st; // FIXME handle symlink (dereflinks)
  std::cout << p << std::endl;
  auto s = dir / p;
  ctx.nc.render_image(s.c_str(), ctx.alignment, NCBLIT_DEFAULT, NCSCALE_SCALE);
  return 0;
}

// if |directories| is true, only print details of |p|, and return. otherwise,
// if |recursedirs| or |toplevel| is set, we will recurse, passing false as
// toplevel (but preserving |recursedirs|).
static int
handle_dir(int dirfd, std::filesystem::path& pdir, const char* p, const struct stat* st, const lsContext& ctx, bool toplevel){
  if(ctx.directories){
    return handle_inode(pdir, p, st, ctx);
  }
  if(!ctx.recursedirs && !toplevel){
    return handle_inode(pdir, p, st, ctx);
  }
  if((strcmp(p, ".") == 0 || strcmp(p, "..") == 0) && !toplevel){
    return 0;
  }
  int newdir = openat(dirfd, p, O_DIRECTORY | O_CLOEXEC);
  if(newdir < 0){
    std::cerr << "Error opening " << p << ": " << strerror(errno) << std::endl;
    return -1;
  }
  DIR* dir = fdopendir(newdir);
  auto subdir = pdir / p;
  if(dir == NULL){
    std::cerr << "Error opening " << p << ": " << strerror(errno) << std::endl;
    close(newdir);
    return -1;
  }
  struct dirent* dent;
  int r = 0;
  while(errno = 0, (dent = readdir(dir))){
    r |= handle_path(newdir, subdir, dent->d_name, ctx, false);
  }
  if(errno){
    std::cerr << "Error reading from " << p << ": " << strerror(errno) << std::endl;
    closedir(dir);
    close(newdir);
    return -1;
  }
  closedir(dir);
  close(newdir);
  return 0;
}

static int
handle_deref(const char* p, const struct stat* st, const lsContext& ctx){
  (void)p;
  (void)st;
  (void)ctx; // FIXME dereference and rerun on target
  return 0;
}

// handle some path |p|, either absolute or relative to |dirfd|. |toplevel| is
// true iff the path was directly listed on the command line.
static int
handle_path(int dirfd, std::filesystem::path& pdir, const char* p, const lsContext& ctx, bool toplevel){
  struct stat st;
  if(fstatat(dirfd, p, &st, AT_NO_AUTOMOUNT)){
    std::cerr << "Error running fstatat(" << p << "): " << strerror(errno) << std::endl;
    return -1;
  }
  if((st.st_mode & S_IFMT) == S_IFDIR){
    return handle_dir(dirfd, pdir, p, &st, ctx, toplevel);
  }else if((st.st_mode & S_IFMT) == S_IFLNK){
    if(toplevel && ctx.dereflinks){
      return handle_deref(p, &st, ctx);
    }
  }
  return handle_inode(pdir, p, &st, ctx);
}

// these are our command line arguments. they're the only paths for which
// handle_path() gets toplevel == true.
static int
list_paths(const char* const * argv, const lsContext& ctx){
  int dirfd = open(".", O_DIRECTORY | O_CLOEXEC);
  if(dirfd < 0){
    std::cerr << "Error opening current directory: " << strerror(errno) << std::endl;
    return -1;
  }
  int ret = 0;
  while(*argv){
    std::filesystem::path s;
    ret |= handle_path(dirfd, s, *argv, ctx, true);
    ++argv;
  }
  close(dirfd);
  return ret;
}

int main(int argc, char* const * argv){
  bool directories = false;
  bool recursedirs = false;
  bool longlisting = false;
  bool dereflinks = false;
  ncalign_e alignment = NCALIGN_RIGHT;
  const struct option opts[] = {
    { "align", 1, NULL, 'a' },
    { NULL, 0, NULL, 0 },
  };
  int c, lidx;
  while((c = getopt_long(argc, argv, "a:dhlLR", opts, &lidx)) != -1){
    switch(c){
      case 'a':
        if(strcasecmp(optarg, "left") == 0){
          alignment = NCALIGN_LEFT;
        }else if(strcasecmp(optarg, "right") == 0){
          alignment = NCALIGN_RIGHT;
        }else if(strcasecmp(optarg, "center") == 0){
          alignment = NCALIGN_CENTER;
        }else{
          std::cerr << "Unknown alignment type: " << optarg << "\n";
          usage(std::cerr, argv[0], EXIT_FAILURE);
        }
        break;
      case 'd':
        directories = true;
        break;
      case 'l':
        longlisting = true;
        break;
      case 'L':
        dereflinks = true;
        break;
      case 'R':
        recursedirs = true;
        break;
      case 'h':
        usage(std::cout, argv[0], EXIT_SUCCESS);
        break;
      default:
        usage(std::cerr, argv[0], EXIT_FAILURE);
        break;
    }
  }
  lsContext ctx = {
    .nc = ncpp::Direct(),
    .longlisting = longlisting,
    .recursedirs = recursedirs,
    .directories = directories,
    .dereflinks = dereflinks,
    .alignment = alignment,
  };
  static const char* const default_args[] = { ".", nullptr };
  list_paths(argv[optind] ? argv + optind : default_args, ctx);
  return EXIT_SUCCESS;
}
