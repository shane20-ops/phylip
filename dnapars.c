/* Version 4.0 (c) Copyright 1993-2013 by the University of Washington.
   Written by Joseph Felsenstein, Akiko Fuseki, Sean Lamont, and Andrew Keeffe.
   Permission is granted to copy and use this program provided no fee is
   charged for it and provided that this copyright notice is not removed. */


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "phylip.h"
#include "seq.h"
#include "dnaparsimony.h"

extern long nextree;    /* parsimony.c -- for counting stored trees */

#ifndef OLDC
/* function prototypes */
void    getoptions(void);
void    allocrest(void);
void    reallocchars(void);
void    doinit(void);
void    makeweights(void);
void    doinput(void);
void    describe(void);
void    maketree(void);
void    freerest(void);
void    dnaparsrun(void);
void    dnapars(char *infilename, char *intreename, char *outfilename, char *outfileopt, char *weightsfilename,
                char *outtreename, char *outtreeopt, int searchbest, char *searchopts, int treesave, int inputorder,
                int randnum, int numjumble, int outroot, int outnum, int threshpars, double threshval, int transpars,
                int sitesweighted, int analyzemult, int multdataset, int nummult, int inputseq, int doprintdata,
                int dodotdiff, int printprog, int printtree, int printsteps, int printseq, int writetree);
/* function prototypes */
#endif

long jumb = 0, nonodes = 0;
Char infilename[FNMLNGTH], outfilename[FNMLNGTH], intreename[FNMLNGTH], outtreename[FNMLNGTH], weightfilename[FNMLNGTH];
char basechar[32]="ACMGRSVTWYHKDBNO???????????????";
long chars, col, msets, ith, njumble;
/*   chars = number of sites in actual sequences */
long inseed, inseed0;
double threshold;
boolean jumble, thresh, weights, thorough, rearrfirst, trout, progress, stepbox, ancseq, mulsets, justwts, firstset, multf;
extern boolean usertree;
steptr oldweight;
longer seed;
tree* curtree;

/* Local variables for Pascal maketree, propagated globally for C version: */
extern double *threshwt;
extern long minwhich;
extern boolean lastrearr, recompute, mulf;
extern double nsteps[maxuser], minsteps, like;
extern long **fsteps;
// extern node *temp, *temp1, *temp2, *tempsum, *temprm, *tempadd, *tempf, *tmp, *tmp1, *tmp2, *tmp3, *tmprm, *tmpadd;
extern double bestyet, bestlike, bstlike2;
extern bestelm *bestrees;
extern long *place;
extern long maxtrees;

node *there;
baseptr nothing;
boolean *names;
char *progname;


void getoptions(void)
{
  /* interactively set options */
  long loopcount, loopcount2;
  Char ch, ch2;

  jumble = false;
  njumble = 1;
  outgrno = 1;
  outgropt = false;
  thresh = false;
  thorough = true;
  transvp = false;
  rearrfirst = false;
  maxtrees = 10000;
  trout = true;
  usertree = false;
  weights = false;
  mulsets = false;
  printdata = false;
  progress = true;
  treeprint = true;
  stepbox = false;
  ancseq = false;
  dotdiff = true;
  interleaved = true;
  justwts = false;
  loopcount = 0;
  for (;;)
  {
    cleerhome();
    printf("\nDNA parsimony algorithm, version %s\n\n", VERSION);
    printf("Setting for this run:\n");
    printf("  U                 Search for best tree?  %s\n",
           (usertree ? "No, use user trees in input file" : "Yes"));
    if (!usertree)
    {
      printf("  S                        Search option?  ");
      if (thorough)
        printf("More thorough search\n");
      else if (rearrfirst)
        printf("Rearrange on one best tree\n");
      else
        printf("Less thorough\n");
      printf("  V              Number of trees to save?  %ld\n", maxtrees);
      printf("  J   Randomize input order of sequences?");
      if (jumble)
        printf("  Yes (seed =%8ld,%3ld times)\n", inseed0, njumble);
      else
        printf("  No. Use input order\n");
    }
    printf("  O                        Outgroup root?");
    if (outgropt)
      printf("  Yes, at sequence number%3ld\n", outgrno);
    else
      printf("  No, use as outgroup species%3ld\n", outgrno);
    printf("  T              Use Threshold parsimony?");
    if (thresh)
      printf("  Yes, count steps up to%4.1f per site\n", threshold);
    else
      printf("  No, use ordinary parsimony\n");
    printf("  N           Use Transversion parsimony?");
    if (transvp)
      printf("  Yes, count only transversions\n");
    else
      printf("  No, count all steps\n");
    printf("  W                       Sites weighted?  %s\n",
           (weights ? "Yes" : "No"));
    printf("  M           Analyze multiple data sets?");
    if (mulsets)
      printf("  Yes, %2ld %s\n", msets,
             (justwts ? "sets of weights" : "data sets"));
    else
      printf("  No\n");
    printf("  I          Input sequences interleaved?  %s\n",
           (interleaved ? "Yes" : "No, sequential"));
    printf("  0   Terminal type (IBM PC, ANSI, none)?  %s\n",
           ibmpc ? "IBM PC" : ansi  ? "ANSI"  : "(none)");
    printf("  1    Print out the data at start of run  %s\n",
           (printdata ? "Yes" : "No"));
    printf("  2  Print indications of progress of run  %s\n",
           progress ? "Yes" : "No");
    printf("  3                        Print out tree  %s\n",
           treeprint ? "Yes" : "No");
    printf("  4          Print out steps in each site  %s\n",
           stepbox ? "Yes" : "No");
    printf("  5  Print sequences at all nodes of tree  %s\n",
           ancseq ? "Yes" : "No");
    if (ancseq || printdata)
      printf("  .  Use dot-differencing to display them  %s\n",
             dotdiff ? "Yes" : "No");
    printf("  6       Write out trees onto tree file?  %s\n",
           trout ? "Yes" : "No");
    printf("\n  Y to accept these or type the letter for one to change\n");
    phyFillScreenColor();
    if(scanf("%c%*[^\n]", &ch)) {}      // Read char and scan to EOL.
    (void)getchar();
    if (ch == '\n')
      ch = ' ';
    uppercase(&ch);
    if (ch == 'Y')
      break;
    if (((!usertree) && (strchr("WSVJOTNUMI12345.60", ch) != NULL))
        || (usertree && ((strchr("WSVOTNUMI12345.60", ch) != NULL))))
    {
      switch (ch)
      {
        case 'J':
          jumble = !jumble;
          if (jumble)
            initjumble(&inseed, &inseed0, seed, &njumble);
          else njumble = 1;
          break;

        case 'O':
          outgropt = !outgropt;
          if (outgropt)
            initoutgroup(&outgrno, spp);
          break;

        case 'T':
          thresh = !thresh;
          if (thresh)
            initthreshold(&threshold);
          break;

        case 'N':
          transvp = !transvp;
          break;

        case 'W':
          weights = !weights;
          break;

        case 'M':
          mulsets = !mulsets;
          if (mulsets)
          {
            printf("Multiple data sets or multiple weights?");
            loopcount2 = 0;
            do {
              printf(" (type D or W)\n");
              phyFillScreenColor();
              if(scanf("%c%*[^\n]", &ch2)) {} // Read char and scan to EOL.
              (void)getchar();
              if (ch2 == '\n')
                ch2 = ' ';
              uppercase(&ch2);
              countup(&loopcount2, 10);
            } while ((ch2 != 'W') && (ch2 != 'D'));
            justwts = (ch2 == 'W');
            if (justwts)
              justweights(&msets);
            else
              initdatasets(&msets);
            if (!jumble)
            {
              jumble = true;
              initjumble(&inseed, &inseed0, seed, &njumble);
            }
          }
          break;

        case 'U':
          usertree = !usertree;
          break;

        case 'S':
          thorough = !thorough;
          if (!thorough)
          {
            printf("Rearrange on just one best tree?");
            loopcount2 = 0;
            do {
              printf(" (type Y or N)\n");
              phyFillScreenColor();
              if(scanf("%c%*[^\n]", &ch2)) {} // Read char and scan to EOL.
              (void)getchar();
              if (ch2 == '\n')
                ch2 = ' ';
              uppercase(&ch2);
              countup(&loopcount2, 10);
            } while ((ch2 != 'Y') && (ch2 != 'N'));
            rearrfirst = (ch2 == 'Y');
          }
          break;

        case 'V':
          loopcount2 = 0;
          do {
            printf("type the number of trees to save\n");
            phyFillScreenColor();
            if(scanf("%ld%*[^\n]", &maxtrees)) {} // Read number and scan to EOL.
            if (maxtrees > MAXNUMTREES)
              maxtrees = MAXNUMTREES;
            (void)getchar();
            countup(&loopcount2, 10);
          } while (maxtrees < 1);
          break;

        case 'I':
          interleaved = !interleaved;
          break;

        case '0':
          initterminal(&ibmpc, &ansi);
          break;

        case '1':
          printdata = !printdata;
          break;

        case '2':
          progress = !progress;
          break;

        case '3':
          treeprint = !treeprint;
          break;

        case '4':
          stepbox = !stepbox;
          break;

        case '5':
          ancseq = !ancseq;
          break;

        case '.':
          dotdiff = !dotdiff;
          break;

        case '6':
          trout = !trout;
          break;
      }
    }
    else
      printf("Not a possible option!\n");
    countup(&loopcount, 100);
  }
  if (transvp)
    fprintf(outfile, "Transversion parsimony\n\n");
}  /* getoptions */


void allocrest(void)
{
  long i;

  inputSequences = (Char **)Malloc(spp * sizeof(Char *));
  for (i = 0; i < spp; i++)
    inputSequences[i] = (Char *)Malloc(chars * sizeof(Char));
  bestrees = (bestelm *)Malloc(maxtrees * sizeof(bestelm));
  for (i = 1; i <= maxtrees; i++)
    bestrees[i - 1].btree = (long *)Malloc(nonodes * sizeof(long));
  nayme = (naym *)Malloc(spp * sizeof(naym));
  place = (long *)Malloc(nonodes * sizeof(long));
  weight = (long *)Malloc(chars * sizeof(long));
  oldweight = (long *)Malloc(chars * sizeof(long));
  alias = (long *)Malloc(chars * sizeof(long));
  ally = (long *)Malloc(chars * sizeof(long));
  location = (long *)Malloc(chars * sizeof(long));
}  /* allocrest */


void doinit(void)
{
  /* initializes variables */
  fprintf(outfile, "\nDNA parsimony algorithm, version %s\n\n", VERSION);

  //inputnumbers(&spp, &chars, &nonodes, 2);
  inputnumbers(&spp, &chars, &nonodes, 1);
  if (!javarun)
  {
    getoptions();
  }
  if (printdata)
    fprintf(outfile, "%2ld species, %3ld  sites\n\n", spp, chars);
}  /* doinit */


void doinput(void)
{
  /* reads the input data */
  long i;

  if (justwts)
  {
    if (firstset)
      inputdata(chars);
    for (i = 0; i < chars; i++)
      weight[i] = 1;
    inputweights(chars, weight, &weights);
    if (justwts)
    {
      fprintf(outfile, "\n\nWeights set # %ld:\n\n", ith);
      if (progress)
      {
        sprintf(progbuf, "\nWeights set # %ld:\n\n", ith);
        print_progress(progbuf);
      }
    }
    if (printdata)
      printweights(outfile, 0, chars, weight, "Sites");
  }
  else
  {
    if (!firstset)
    {
      samenumsp(&chars, ith);
      reallocchars();
    }
    inputdata(chars);
    for (i = 0; i < chars; i++)
      weight[i] = 1;
    if (weights)
    {
      inputweights(chars, weight, &weights);
      if (printdata)
        printweights(outfile, 0, chars, weight, "Sites");
    }
  }
  makeweights();
  curtree = dnapars_tree_new(nonodes, spp);
  dna_makevalues(curtree, usertree);
}  /* doinput */


void makeweights(void)
{
  /* make up weights vector to avoid duplicate computations */
  long i;

  for (i = 1; i <= chars; i++)
  {
    alias[i - 1] = i;
    oldweight[i - 1] = weight[i - 1];
    ally[i - 1] = i;
  }
  sitesort(chars, weight);
  sitecombine(chars);
  sitescrunch(chars);
  endsite = 0;
  for (i = 1; i <= chars; i++)
  {
    if (ally[i - 1] == i)
      endsite++;
  }
  for (i = 1; i <= endsite; i++)
    location[alias[i - 1] - 1] = i;
  if (!thresh)
    threshold = spp;
  threshwt = (double *)Malloc(endsite * sizeof(double));
  for (i = 0; i < endsite; i++)
  {
    threshwt[i] = (threshold * weight[i]);
  }
}  /* makeweights */


void describe(void)
{
  /* prints ancestors, steps and table of numbers of steps in
     each site */
  if (treeprint)
  {
    fprintf(outfile, "\nrequires a total of %10.3f\n", -(curtree->score));
    fprintf(outfile, "\n  between      and       length\n");
    fprintf(outfile, "  -------      ---       ------\n");
    printbranchlengths(curtree->root);
  }
  if (stepbox)
    writesteps(curtree, chars, weights, oldweight);
  if (ancseq)
  {
    dna_hypstates(curtree, chars, basechar);
    putc('\n', outfile);
  }
  putc('\n', outfile);
  if (trout)
  {
    col = 0;
    treeout3(curtree->root, nextree, &col, curtree->root);
  }
}  /* describe */


void maketree(void)                     // RSGbugfix
{
  /* constructs a binary tree from the pointers in treenode.
     adds each node at location which yields highest "likelihood"
     then rearranges the tree for greatest "likelihood" */
  long i, j, nextnode;
  boolean done, firsttree, goteof, haslengths;

#if 0                                   // RSGbugfix: Local variable never used.
  pointarray nodep;
#endif

  // RSGnote: Was formerly NOT INITIALIZED here and then potentially referenced
  // before being initialized in one branch of IF below.  This is a bug unless we can
  // prove that the appropriate branch of that IF will always be taken.  Initialized
  // here only to silence compiler warning.
  long numtrees = 0;

  if (!usertree)
  {
    lastrearr = false;
    hsbut(curtree, false, jumble, seed, progress);
    if (progress)
    {
      sprintf(progbuf, "\nDoing global rearrangements");
      print_progress(progbuf);
      if (rearrfirst)
      {
        sprintf(progbuf, " on the first of the trees tied for best\n");
      }
      else
      {
        sprintf(progbuf, " on all trees tied for best\n");
      }
      print_progress(progbuf);
      sprintf(progbuf, "  !");
      print_progress(progbuf);
      for (j = 0; j < nonodes; j++)
      {
        if (j % ((nonodes / 72) + 1) == 0)
        {
          sprintf(progbuf, "-");
          print_progress(progbuf);
        }
      }
      sprintf(progbuf, "!\n");
      print_progress(progbuf);
      phyFillScreenColor();
    }

    done = false;
    (void)done;                         // RSGnote: Variable set but never used.

    lastrearr = true;
    grandrearr(curtree, progress, rearrfirst);

    if (progress)
    {
      sprintf(progbuf, "\n");
      print_progress(progbuf);
    }
    if (jumb == njumble)
    {
      long outCount = 0;
      collapsebestrees(curtree, bestrees, place, chars, progress, &outCount);
      long missedCount = nextree - 1 - maxtrees;
      if (treeprint)
      {
        putc('\n', outfile);
        if (outCount == 1)
          fprintf(outfile, "One most parsimonious tree found:\n");
        else
        {
          if (missedCount > 0)
          {
            fprintf(outfile, "as many as %ld trees may have been found\n", missedCount + outCount);
            fprintf(outfile, "here are the first %4ld of them\n", outCount );
          }
          else
          {
            fprintf(outfile, "%6ld trees in all found\n", outCount);
          }

        }
      }
      if (treeprint)
        putc('\n', outfile);
      for (i = 0; i < outCount ; i++)
      {
        load_tree(curtree, i, bestrees);
        reroot(curtree->nodep[outgrno - 1], curtree->root);
        curtree->evaluate(curtree, curtree->root, false);
        curtree->root = root_tree(curtree, curtree->root);
        curtree->nodep[curtree->root->index - 1] = curtree->root;
        dna_treelength(curtree->root, chars, curtree->nodep);
        printree(curtree);
        describe();
        reroot_tree(curtree, curtree->root);                // RSGbugfix: Name change.
      }
    }
  }
  else
  {
    /* Open in binary: ftell() is broken for UNIX line-endings under WIN32 */
    openfile(&intree, INTREE, "input tree", "rb", progname, intreename);
    numtrees = countsemic(intree);
    if (numtrees > 2)
      initseed(&inseed, &inseed0, seed);
    if (numtrees > MAXNUMTREES)
    {
      printf("\nERROR:  Number of input trees is read incorrectly from %s.\n", intreename);
      exxit(-1);
    }
    if (treeprint)
    {
      fprintf(outfile, "User-defined tree");
      if (numtrees > 1)
        putc('s', outfile);
      fprintf(outfile, ":\n");
    }
    fsteps = (long **)Malloc(maxuser * sizeof(long *));
    for (j = 1; j <= maxuser; j++)
      fsteps[j - 1] = (long *)Malloc(endsite * sizeof(long));
    if (trout)
      fprintf(outtree, "%ld\n", numtrees);

#if 0                                   // RSGbugfix: Local variable never used.
    nodep = NULL;
#endif

    which = 1;
    while (which <= numtrees)
    {
      firsttree = true;
      nextnode = 0;
      haslengths = true;
      treeread(curtree, intree, &curtree->root, curtree->nodep, &goteof, &firsttree, &nextnode, &haslengths, initparsnode, false, nonodes);
      reroot_tree(curtree, curtree->root);                // RSGbugfix: Name change.
      curtree->evaluate(curtree, curtree->root, false);
      curtree->root = root_tree(curtree, curtree->root);
      if (treeprint)
        fprintf(outfile, "\n\n");
      if (outgropt)
        reroot(curtree->nodep[outgrno - 1], curtree->root);
      dna_treelength(curtree->root, chars, curtree->nodep);
      printree(curtree);
      describe();
      if (which < numtrees)
      {
        // may need to free some memory here
      }
      which++;
    }
    FClose(intree);
    putc('\n', outfile);
    if (numtrees > 1 && chars > 1 )
      standev(chars, numtrees, minwhich, minsteps, nsteps, fsteps, seed);
    for (j = 1; j <= maxuser; j++)
      free(fsteps[j - 1]);
    free(fsteps);
  }
  if (jumb == njumble)
  {
    if (progress)
    {
      sprintf(progbuf, "\nOutput written to file \"%s\".\n\n", outfilename);
      print_progress(progbuf);
      if (trout)
      {
        sprintf(progbuf, "Tree");
        print_progress(progbuf);

        // RSGnote: "numtrees" may be reference before being initialized here.
        if ((usertree && numtrees > 1) || (!usertree && nextree != 2))
        {
          sprintf(progbuf, "s");
          print_progress(progbuf);
        }
        sprintf(progbuf, " also written onto file \"%s\".\n\n", outtreename);
        print_progress(progbuf);
      }
    }
  }
}  /* maketree */


void reallocchars(void)
{ /* The amount of chars can change between runs this function reallocates all the variables whose size depends on the amount of chars */
  long i;

  for (i=0; i < spp; i++)
  {
    free(inputSequences[i]);
    inputSequences[i] = (Char *)Malloc(chars * sizeof(Char));
  }
  free(weight);
  free(oldweight);
  free(alias);
  free(ally);
  free(location);

  weight = (long *)Malloc(chars * sizeof(long));
  oldweight = (long *)Malloc(chars * sizeof(long));
  alias = (long *)Malloc(chars * sizeof(long));
  ally = (long *)Malloc(chars * sizeof(long));
  location = (long *)Malloc(chars * sizeof(long));
}


void freerest(void)
{ /* free variables that are allocated each data set */
  long i;

  for (i = 0; i < spp; i++)
    free(inputSequences[i]);
  free(inputSequences);
  for (i = 1; i <= maxtrees; i++)
    free(bestrees[i - 1].btree);
  free(bestrees);
  free(nayme);
  free(place);
  free(weight);
  free(oldweight);
  free(alias);
  free(ally);
  free(location);
  free(threshwt);
}  /* freerest */


void dnaparsrun(void)
{
  // debug printout // JRMdebug
  /*
  printf("jumble: %i\n", jumble);
  printf("njumble: %li\n", njumble);
  printf("outgrno: %li\n", outgrno);
  printf("outgropt: %i\n", outgropt);
  printf("thresh: %i\n", thresh);
  printf("thorough: %i\n", thorough);
  printf("transvp: %i\n", transvp);
  printf("rearrfirst: %i\n", rearrfirst);
  printf("maxtrees: %li\n", maxtrees);
  printf("trout: %i\n", trout);
  printf("usertree: %i\n", usertree);
  printf("weights: %i\n", weights);
  printf("mulsets: %i\n", mulsets);
  printf("msets: %li\n", msets);
  printf("printdata: %i\n", printdata);
  printf("progress: %i\n", progress);
  printf("treeprint: %i\n", treeprint);
  printf("stepbox: %i\n", stepbox);
  printf("ancseq: %i\n", ancseq);
  printf("dotdiff: %i\n", dotdiff);
  printf("interleaved: %i\n", interleaved);
  printf("justwts: %i\n", justwts);
*/
  for (ith = 1; ith <= msets; ith++) {
    if (!(justwts && !firstset))
    {
      //printf("calling allocrest\n"); // JRMdebug
      allocrest();
    }
    if (msets > 1 && !justwts) {
      fprintf(outfile, "\nData set # %ld:\n\n", ith);
      if (progress)
      {
        sprintf(progbuf, "\nData set # %ld:\n\n", ith);
        print_progress(progbuf);
      }
    }
    //printf("calling doinput\n"); // JRMdebug
    doinput();
    if (ith == 1)
      firstset = false;
    for (jumb = 1; jumb <= njumble; jumb++)
    {
      //printf("calling maketree\n"); // JRMdebug
      maketree();
    }
    if (!justwts)
      freerest();
    fflush(outfile);
    fflush(outtree);
  }

  curtree->free(curtree);
}


void dnapars(
  char *infilename,
  char *intreename,
  char *OutfileName,
  char *outfileopt,
  char *weightsfilename,
  char *OuttreeName,
  char *outtreeopt,
  int searchbest,
  char *searchopts,
  int treesave,
  int inputorder,
  int randnum,
  int numjumble,
  int outroot,
  int outnum,
  int threshpars,
  double threshval,
  int transpars,
  int sitesweighted,
  int analyzemult,
  int multdataset,
  int nummult,
  int inputseq,
  int doprintdata,
  int dodotdiff,
  int printprog,
  int printtree,
  int printsteps,
  int printseq,
  int writetree
  )
{
  initdata funcs;

  //printf("Hello from DnaPars!\n"); // JRMdebug

  int argc;
  Char *argv[1];
  argc = 1;
  argv[0] = "Dnapars";

  memset(&funcs, 0, sizeof(funcs));
  funcs.node_new = dnapars_node_new;
  funcs.tree_new = dnapars_tree_new;

  phylipinit(argc, argv, &funcs, true);


  /* // JRMdebug
  // internal variables
  //  jumble = false;
  //  njumble = 1;
  //  outgrno = 1;
  //  outgropt = false;
  //  thresh = false;
  //  thorough = true;
  //  transvp = false;
  //  rearrfirst = false;
  //  maxtrees = 10000;
  //  trout = true;
  //  usertree = false;
  //  weights = false;
  //  mulsets = false;
  //  printdata = false;
  //  progress = true;
  //  treeprint = true;
  //  stepbox = false;
  //  ancseq = false;
  //  dotdiff = true;
  //  interleaved = true;
  //  justwts = false;

  // from java
  //    char *infilename,
  //    char *intreename,
  //    char *outfilename,
  //    char *outfileopt,
  //    char *weightsfilename,
  //    char *outtreename,
  //    int searchbest,
  //    char *searchopts,
  //    int treesave,
  //    int inputorder,
  //    int randnum,
  //    int numjumble,
  //    int outroot,
  //    int outnum,
  //    int threshpars,
  //    double threshval,
  //    int transpars,
  //    int sitesweighted,
  //    int analyzemult,
  //    int multdataset,
  //    int inputseq,
  //    int doprintdata,
  //    int dodotdiff,
  //    int printprog,
  //    int printtree,
  //    int printsteps,
  //    int printseq,
  //    int writetree

  */

  // transfer java data to local variables
  if (searchbest != 0)
  {
    usertree = false;
  }
  else
  {
    usertree = true;
  }

  if (strcmp(searchopts, "More"))
  {
    thorough   = true;
    rearrfirst = false;
  }
  else if (!strcmp(searchopts, "Less"))
  {
    thorough   = false;
    rearrfirst = false;
  }
  else // Rearrange
  {
    thorough   = false;
    rearrfirst = true;
  }

  maxtrees = treesave;

  if (inputorder != 0)
  {
    jumble = true;
  }
  else
  {
    jumble = false;
  }

  inseed =  randnum;

  njumble = numjumble;

  outgrno = outnum;

  if (outroot != 0)
  {
    outgropt = true;
    initoutgroup(&outgrno, spp);
  }
  else
  {
    outgropt = false;
  }

  if (threshpars != 0)
  {
    thresh = true;
  }
  else
  {
    thresh = false;
  }

  threshold = threshval;

  if (transpars != 0)
  {
    transvp = true;
  }
  else
  {
    transvp = false;
  }

  if (sitesweighted != 0)
  {
    weights = true;
  }
  else
  {
    weights = false;
  }

  if (analyzemult !=  0)
  {
    mulsets = true;
  }
  else
  {
    mulsets = false;
  }

  msets = nummult;

  if (multdataset != 0)
  {
    justwts = false;
  }
  else
  {
    justwts = true;
  }

  if (inputseq != 0)
  {
    interleaved =true;
  }
  else
  {
    interleaved = false;
  }

  if (doprintdata != 0)
  {
    printdata = true;
  }
  else
  {
    printdata = false;
  }

  if (dodotdiff != 0)
  {
    dotdiff = true;
  }
  else
  {
    dotdiff = false;
  }

  if (printprog != 0)
  {
    progress = true;
  }
  else
  {
    progress = false;
  }

  if (printtree != 0)
  {
    trout = true;
  }
  else
  {
    trout = false;
  }

  if (printsteps != 0)
  {
    stepbox = true;
  }
  else
  {
    stepbox = false;
  }

  if (printseq != 0)
  {
    ancseq = true;
  }
  else
  {
    ancseq = false;
  }

  if (writetree != 0)
  {
    treeprint = true;
  }
  else
  {
    treeprint = false;
  }

  // everything translated, start the run
  infile = fopen(infilename, "r");
  outfile = fopen(OutfileName, outfileopt);
  strcpy(outfilename, OutfileName);
  if (usertree)
  {
    intree = fopen(intreename, "r");
  }

  if (progress)
  {
    progfile = fopen("progress.txt", "w");
    fclose(progfile); // make sure it is there for the Java code to detect
    progfile = fopen("progress.txt", "w");
  }

  firstset = true;
  doinit();

  if (weights || justwts)
  {
    weightfile = fopen(weightsfilename, "r");
  }

  if (trout)
  {
    outtree = fopen(OuttreeName, outtreeopt);
    strcpy(outtreename, OuttreeName);
  }

  if (jumble) {
    initjumble(&inseed, &inseed0, seed, &njumble);
  }

  dnaparsrun();  // do the actual work

  FClose(infile);
  FClose(outfile);

  if (weights || justwts)
  {
    FClose(weightfile);
  }

  if (trout)
  {
    FClose(outtree);
  }

  if (usertree)
  {
    FClose(intree);
  }

  //printf("\ndone\n"); // JRMdebug
}


int main(int argc, Char *argv[])
{  /* DNA parsimony by uphill search */

  /* reads in spp, chars, and the data. Then calls maketree to
     construct the tree */
  initdata funcs;
#ifdef MAC
  argc = 1;        /* macsetup("Dnapars", "");        */
  argv[0] = "Dnapars";
#endif
  memset(&funcs, 0, sizeof(funcs));
  funcs.node_new = dnapars_node_new;
  funcs.tree_new = dnapars_tree_new;
  phylipinit(argc, argv, &funcs, false);
  progname = argv[0];
  openfile(&infile, INFILE, "input file", "r", argv[0], infilename);
  openfile(&outfile, OUTFILE, "output file", "w", argv[0], outfilename);

  ibmpc = IBMCRT;
  ansi = ANSICRT;
  msets = 1;
  firstset = true;
  doinit();
  if (weights || justwts)
    openfile(&weightfile, WEIGHTFILE, "weights file", "r", argv[0], weightfilename);
  if (trout)
    openfile(&outtree, OUTTREE, "output tree file", "w", argv[0], outtreename);

  dnaparsrun();  // do the actual work

  FClose(infile);
  FClose(outfile);
  if (weights || justwts)
    FClose(weightfile);
  if (trout)
    FClose(outtree);
  if (usertree)
    FClose(intree);
#ifdef MAC
  fixmacfile(outfilename);
  fixmacfile(outtreename);
#endif
  if (progress)
  {
    printf("Done.\n\n");
  }
  phyRestoreConsoleAttributes();
  return 0;
}  /* DNA parsimony by uphill search */


// End.
