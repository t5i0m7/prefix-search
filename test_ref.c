#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "tst.h"

/** constants insert, delete, max word(s) & stack nodes */
enum { INS, DEL, WRDMAX = 256, STKMAX = 512, LMAX = 1024 };
#define REF INS
#define CPY DEL

/* timing helper function */
static double tvgetf(void)
{
    struct timespec ts;
    double sec;

    clock_gettime(CLOCK_REALTIME, &ts);
    sec = ts.tv_nsec;
    sec /= 1e9;
    sec += ts.tv_sec;

    return sec;
}
/* delete comma and get the city*/
void rmcomma(char *s)
{
    size_t len = strlen(s);
    for(int i = 0 ; i < len ; i++)
        if(s[i]==',')
            s[i] = 0;

}


/* simple trim '\n' from end of buffer filled by fgets */
static void rmcrlf(char *s)
{
    size_t len = strlen(s);
    if (len && s[len - 1] == '\n')
        s[--len] = 0;
}

#define IN_FILE "cities.txt"
#define CITY_NUM 30000
int main(int argc, char **argv)
{
    char word[CITY_NUM][WRDMAX];
    char *sgl[LMAX] = {NULL};
    tst_node *root = NULL, *res = NULL;
    int rtn = 0, idx = 1, sidx = 0, bench_flag = 0;
    FILE *fp = fopen(IN_FILE, "r"),*fp_search,*fp_tst;//*fp_redo = fopen("cities_redo","w");
    double t1, t2;

    if (!fp) { /* prompt, open, validate file for reading */
        fprintf(stderr, "error: file open failed '%s'.\n", argv[1]);
        return 1;
    }



    t1 = tvgetf();
    for ( ; (rtn = fgets(word[idx], WRDMAX , fp) != NULL ) && idx <= 30000; idx++) {
        rmcrlf(word[idx]);
        rmcomma(word[idx]);
        char *p = word[idx];
        //fprintf(fp_redo,"%s\n",p);
        /* FIXME: insert reference to each string */
        if (!tst_ins_del(&root, &p, INS, REF)) {
            fprintf(stderr, "error: memory exhausted, tst_insert.\n");
            fclose(fp);
            return 1;
        }

    }
    t2 = tvgetf();

    fclose(fp);
    //fclose(fp_redo);

    /* Determine whether a bench mode is */
    if(argc == 2 && strcmp(argv[1],"--bench")) {
        bench_flag = 1;
        fp_tst=fopen("output_tst","a");
        fp_search=fopen("output_search","a");
    }

    /*bench test write file*/
    if(bench_flag) {
        fprintf(fp_tst,"%f\n",t2-t1);
        t1=tvgetf();
        res = tst_search_prefix(root, "Ame", sgl, &sidx, LMAX);
        t2=tvgetf();
        fprintf(fp_search,"%f\n",t2-t1);
        fclose(fp_tst);
        fclose(fp_search);
        return 0;
    }

    printf("ternary_tree, loaded %d words in %.6f sec\n", idx-1, t2 - t1);

    for (;;) {
        char *p;
        printf(
            "\nCommands:\n"
            " a  add word to the tree\n"
            " f  find word in tree\n"
            " s  search words matching prefix\n"
            " d  delete word from the tree\n"
            " q  quit, freeing all data\n\n"
            "choice: ");
        fgets(word[0], sizeof word[0], stdin);
        p = NULL;
        switch (*word[0]) {
        case 'a':
            printf("enter word to add: ");
            if (!fgets(word[idx], sizeof word[idx], stdin)) {
                fprintf(stderr, "error: insufficient input.\n");
                break;
            }
            rmcrlf(word[idx]);
            p = word[idx];
            t1 = tvgetf();
            /* FIXME: insert reference to each string */
            res = tst_ins_del(&root, &p, INS, REF);
            t2 = tvgetf();
            if (res) {
                idx++;
                printf("  %s - inserted in %.6f sec. (%d words in tree)\n",
                       (char *) res, t2 - t1, idx);
            } else
                printf("  %s - already exists in list.\n", (char *) res);
            break;
        case 'f':
            printf("find word in tree: ");
            if (!fgets(word[0], sizeof word[0], stdin)) {
                fprintf(stderr, "error: insufficient input.\n");
                break;
            }
            rmcrlf(word[0]);
            t1 = tvgetf();
            res = tst_search(root, word[0]);
            t2 = tvgetf();
            if (res)
                printf("  found %s in %.6f sec.\n", (char *) res, t2 - t1);
            else
                printf("  %s not found.\n", word[0]);
            break;
        case 's':
            printf("find words matching prefix (at least 1 char): ");
            if (!fgets(word[0], sizeof word[0], stdin)) {
                fprintf(stderr, "error: insufficient input.\n");
                break;
            }
            rmcrlf(word[0]);
            t1 = tvgetf();
            res = tst_search_prefix(root, word[0], sgl, &sidx, LMAX);
            t2 = tvgetf();
            if (res) {
                printf("  %s - searched prefix in %.6f sec\n\n", word[0], t2 - t1);
                for (int i = 0; i < sidx; i++)
                    printf("suggest[%d] : %s\n", i, sgl[i]);
            } else
                printf("  %s - not found\n", word[0]);
            break;
        case 'd':
            printf("enter word to del: ");
            if (!fgets(word[0], sizeof word[0], stdin)) {
                fprintf(stderr, "error: insufficient input.\n");
                break;
            }
            rmcrlf(word[0]);
            p = word[0];
            printf("  deleting %s\n", word[0]);
            t1 = tvgetf();
            /* FIXME: remove reference to each string */
            res = tst_ins_del(&root, &p, DEL, REF);
            t2 = tvgetf();
            if (res)
                printf("  delete failed.\n");
            else {
                printf("  deleted %s in %.6f sec\n", word[0], t2 - t1);
                // fragmention problem
            }
            break;
        case 'q':
            tst_free_all(root);
            return 0;
            break;
        default:
            fprintf(stderr, "error: invalid selection.\n");
            break;
        }
    }

    return 0;
}
