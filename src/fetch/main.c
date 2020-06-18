#include <locale.h>
#include <sys/utsname.h>
#include <notcurses/notcurses.h>

typedef struct distro_info {
  const char* name;            // must match 'lsb_release -i'
  const char* logofile;        // kept at original aspect ratio, lain atop bg
} distro_info;

static distro_info distros[] = {
  {
    .name = "Debian",
    // from desktop-base package
    .logofile = "/usr/share/desktop-base/debian-logos/logo-text-256.png",
  }, {
    .name = NULL,
    .logofile = NULL,
  },
};

static const distro_info*
getdistro(void){
  FILE* p = popen("lsb_release -i", "re");
  if(p == NULL){
    fprintf(stderr, "Error running lsb_release -i (%s)\n", strerror(errno));
    return NULL;
  }
  const distro_info* dinfo = NULL;
  char* buf = malloc(BUFSIZ); // gatesv("BUFSIZ bytes is enough for anyone")
  if(fgets(buf, BUFSIZ, p) == NULL){
    fprintf(stderr, "Error reading from lsb_release -i (%s)\n", strerror(errno));
    fclose(p);
    goto done;
  }
  if(fclose(p)){
    fprintf(stderr, "Error closing pipe (%s)\n", strerror(errno));
    goto done;
  }
  const char* colon = strchr(buf, ':');
  if(colon == NULL){
    goto done;
  }
  const char* distro = ++colon;
  while(*distro && isspace(*distro)){
    ++distro;
  }
  const char* nl = strchr(distro, '\n');
  if(nl == NULL){
    goto done;
  }
  size_t len = nl - distro;
  if(len){
    for(dinfo = distros ; dinfo->name ; ++dinfo){
      if(strncmp(dinfo->name, distro, nl - distro) == 0){
        if(strlen(dinfo->name) == len){
          break;
        }
      }
    }
  }
  if(dinfo->name == NULL){
    dinfo = NULL;
  }

done:
  free(buf);
  return dinfo;
}

static const distro_info*
linux_ncneofetch(void){
  const distro_info* dinfo = getdistro();
  if(dinfo == NULL){
    return NULL;
  }
  return dinfo;
}

typedef enum {
  NCNEO_LINUX,
  NCNEO_FREEBSD,
  NCNEO_UNKNOWN,
} ncneo_kernel_e;

static ncneo_kernel_e
get_kernel(void){
  struct utsname uts;
  if(uname(&uts)){
    fprintf(stderr, "Failure invoking uname (%s)\n", strerror(errno));
    return -1;
  }
  if(strcmp(uts.sysname, "Linux") == 0){
    return NCNEO_LINUX;
  }else if(strcmp(uts.sysname, "FreeBSD") == 0){
    return NCNEO_FREEBSD;
  }
  fprintf(stderr, "Unknown operating system via uname: %s\n", uts.sysname);
  return NCNEO_UNKNOWN;
}

static struct ncplane*
display(struct notcurses* nc, const distro_info* dinfo){
  if(dinfo->logofile){
    nc_err_e err;
    struct ncvisual* ncv = ncvisual_from_file(dinfo->logofile, &err);
    if(ncv == NULL){
      fprintf(stderr, "Error opening logo file at %s\n", dinfo->logofile);
      return NULL;
    }
    struct ncvisual_options vopts = {
      .scaling = NCSCALE_SCALE,
      .blitter = NCBLIT_2x2,
      .n = notcurses_stdplane(nc),
    };
    if(ncvisual_render(nc, ncv, &vopts) == NULL){
      ncvisual_destroy(ncv);
      return NULL;
    }
    ncvisual_destroy(ncv);
  }
  return 0;
}

static const distro_info*
freebsd_ncneofetch(void){
  static const distro_info fbsd = {
    .name = "FreeBSD",
    .logofile = NULL, // FIXME
  };
  return &fbsd;
}

static int
infoplane(struct notcurses* nc){
  const int dimy = ncplane_dim_y(notcurses_stdplane(nc));
  const int planeheight = 8;
  struct ncplane* infop = ncplane_aligned(notcurses_stdplane(nc),
                                          planeheight, 60,
                                          dimy - (planeheight + 1),
                                          NCALIGN_CENTER, NULL);
  if(infop == NULL){
    return -1;
  }
  if(ncplane_perimeter_rounded(infop, 0, 0, 0)){
    return -1;
  }
  return 0;
}

static int
ncneofetch(struct notcurses* nc){
  const distro_info* dinfo = NULL;
  ncneo_kernel_e kern = get_kernel();
  switch(kern){
    case NCNEO_LINUX:
      dinfo = linux_ncneofetch();
      break;
    case NCNEO_FREEBSD:
      dinfo = freebsd_ncneofetch();
      break;
    case NCNEO_UNKNOWN:
      break;
  }
  if(dinfo == NULL){
    return -1;
  }
  if(display(nc, dinfo)){
    return -1; // FIXME soldier on, perhaps?
  }
  if(infoplane(nc)){
    return -1;
  }
  if(notcurses_render(nc)){
    return -1;
  }
  return 0;
}

int main(void){
  if(setlocale(LC_ALL, "") == NULL){
    fprintf(stderr, "Warning: couldn't set locale based off LANG\n");
  }
  struct notcurses_options nopts = {
    .flags = NCOPTION_INHIBIT_SETLOCALE | NCOPTION_NO_ALTERNATE_SCREEN |
              NCOPTION_SUPPRESS_BANNERS,
  };
  struct notcurses* nc = notcurses_init(&nopts, NULL);
  if(nc == NULL){
    return EXIT_FAILURE;
  }
  int r = ncneofetch(nc);
  if(notcurses_stop(nc)){
    return EXIT_FAILURE;
  }
  return r ? EXIT_FAILURE : EXIT_SUCCESS;
}