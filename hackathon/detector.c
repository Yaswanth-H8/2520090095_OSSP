#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

typedef struct {
    int pid, ppid, tty, session;
    char state;
    char name[256];
} Proc;

static Proc *procs = NULL;
static int n = 0;
static int show_all = 0;

static Proc *find(int pid) {
    for (int i = 0; i < n; i++)
        if (procs[i].pid == pid) return &procs[i];
    return NULL;
}

/* Read /proc/<pid>/stat for every process */
static void read_procs(void) {
    DIR *d = opendir("/proc");
    if (!d) { perror("opendir /proc"); exit(1); }
    int cap = 256;
    procs = malloc(cap * sizeof(Proc));
    struct dirent *e;

    while ((e = readdir(d))) {
        if (!isdigit((unsigned char)e->d_name[0])) continue;

        char path[300], buf[1024];
        snprintf(path, sizeof path, "/proc/%s/stat", e->d_name);
        FILE *f = fopen(path, "r");
        if (!f) continue;                      /* process may have exited */
        size_t len = fread(buf, 1, sizeof buf - 1, f);
        fclose(f);
        buf[len] = '\0';

        /* name is inside ( ) and may contain spaces, so use last ')' */
        char *l = strchr(buf, '('), *r = strrchr(buf, ')');
        if (!l || !r) continue;

        Proc p;
        memset(&p, 0, sizeof p);
        p.pid = atoi(e->d_name);
        int nl = (int)(r - l - 1);
        if (nl > 255) nl = 255;
        memcpy(p.name, l + 1, nl);
        p.name[nl] = '\0';

        int pgrp;
        if (sscanf(r + 2, "%c %d %d %d %d",
                   &p.state, &p.ppid, &pgrp, &p.session, &p.tty) != 5)
            continue;

        if (n == cap) { cap *= 2; procs = realloc(procs, cap * sizeof(Proc)); }
        procs[n++] = p;
    }
    closedir(d);
}

static int is_orphan(Proc *p) {
    if (p->state == 'Z' || p->pid == 1) return 0;
    Proc *par = find(p->ppid);
    int adopted = (p->ppid == 1) ||
                  (par && (strcmp(par->name, "systemd") == 0 ||
                           strcmp(par->name, "init") == 0 ||
                           strncmp(par->name, "Relay", 5) == 0));
    if (!adopted) return 0;
    if (show_all) return 1;
    /* daemons are session leaders; real orphans keep their shell's session */
    return p->tty != 0 && p->session != p->pid;
}

static const char *tag(Proc *p) {
    if (p->state == 'Z') return "  <-- ZOMBIE";
    if (is_orphan(p))    return "  <-- ORPHAN";
    return "";
}

static void print_tree(int pid, const char *prefix, int last, int root) {
    Proc *p = find(pid);
    if (!p) return;

    printf("%s%s[%d] %s (%c)%s\n", prefix,
           root ? "" : (last ? "└── " : "├── "),
           p->pid, p->name, p->state, tag(p));

    char np[512];
    snprintf(np, sizeof np, "%s%s", prefix,
             root ? "" : (last ? "    " : "│   "));

    int total = 0, seen = 0;
    for (int i = 0; i < n; i++)
        if (procs[i].ppid == pid && procs[i].pid != pid) total++;
    for (int i = 0; i < n; i++)
        if (procs[i].ppid == pid && procs[i].pid != pid)
            print_tree(procs[i].pid, np, ++seen == total, 0);
}

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--all") == 0) show_all = 1;

    read_procs();

    int zc = 0, oc = 0;
    for (int i = 0; i < n; i++) {
        if (procs[i].state == 'Z') zc++;
        if (is_orphan(&procs[i])) oc++;
    }

    printf("\n=== ZOMBIE PROCESSES (%d) ===\n", zc);
    for (int i = 0; i < n; i++) {
        Proc *z = &procs[i];
        if (z->state != 'Z') continue;
        Proc *par = find(z->ppid);
        printf("PID %d (%s)  parent: PID %d (%s)\n",
               z->pid, z->name, z->ppid, par ? par->name : "?");
        print_tree(z->ppid, "", 1, 1);
        printf("\n");
    }

    printf("=== ORPHAN PROCESSES (%d) ===\n", oc);
    for (int i = 0; i < n; i++) {
        Proc *p = &procs[i];
        if (!is_orphan(p)) continue;
        Proc *par = find(p->ppid);
        printf("PID %d (%s)  adopted by PID %d (%s)\n",
               p->pid, p->name, p->ppid, par ? par->name : "?");
        print_tree(p->pid, "", 1, 1);
        printf("\n");
    }

    free(procs);
    return 0;
}
