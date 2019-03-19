#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <iostream>
#include <cstring>

#ifndef MEDITH_HXX
#define MEDITH_HXX

#define E_IDX_NONE unsigned int(-1)

#include "Array.h"

using CoordArray = Array<double>;
using IntArr = Array<int>;

class medith
{
public:
  enum eType { NONE = -2, VERT = -1, EDGE, TRI, QUAD, TET, HEX};
  enum eColor { COL_NONE = 0, RED = 1, GREEN = 2, YELLOW = 3};

public:
  static int read (const char* filename, CoordArray& pos, IntArr& connectE2, IntArr& connectT3, IntArr& connectQ4, IntArr& connectTH4, IntArr& connectHX8)
  {
  std::string                  line, entity;
  char cline[512];
  char* words[10];
  std::ifstream                file (filename);
  FILE * fp = fopen(filename, "r");
  int                          nb_entities, i, nb_read;
  
  double                       P[3];
  int                          nb_nodes[5], S[8], nods;

  pos.clear();
  connectE2.clear();
  connectT3.clear();
  connectQ4.clear();
  connectTH4.clear();
  connectHX8.clear();

  if (fp == NULL)
    return 1;

  nb_nodes[EDGE] = 2;
  nb_nodes[TRI] = 3;
  nb_nodes[QUAD] = nb_nodes[TET] = 4;
  nb_nodes[HEX] = 8;

  eType curType(NONE);

  curType = NONE;
  
  while (fgets(cline, 512, fp) != NULL)
  {
    line = cline;
    if (line.empty())
      continue;
            
    get_words(line, ' ', &words[0]);

    if ((curType == VERT) && (nb_read < nb_entities))
    {
      for (i = 0; i < 3; ++i)
        P[i] = fast_atof(words[i]);

      pos.add(P);
      ++nb_read;
      continue;
    }

    if ((curType != NONE) && (nb_read < nb_entities))
    {
      nods = nb_nodes[curType];
      for (i = 0; i < nods; ++i)
        S[i] = fast_atoindex(words[i])-1;

      switch (curType)
      {
      case EDGE:connectE2.add(S); break;
      case TRI:connectT3.add(S); break;
      case QUAD:connectQ4.add(S); break;
      case TET:connectTH4.add(S); break;
      case HEX:connectHX8.add(S); break;
      default:break;
      }

      ++nb_read;
      continue;
    }
    
    entity = words[0];
    curType = NONE;

    if (entity == "End\n")
        break;
    if (entity == "Dimension\n")
    {
      fgets(cline, 512, fp);
      //dim = fast_atoindex(cline);
		  continue;
    }
    if ( (entity == "Vertices\n") || (entity == "Edges\n") || (entity == "Triangles\n") || (entity == "Quadrilaterals\n") || (entity == "Tetrahedra\n") || (entity == "Hexahedra\n") )
    {      
      fgets(cline, 512, fp);
      line = cline;
      get_words(line, ' ', words);

      nb_entities = atoi(words[0]);
      nb_read=0;

      if (entity == "Vertices\n")
      {
        pos.reserve(nb_entities);
        curType = VERT;
      }
      else if (entity == "Edges\n")
      {
        connectE2.reserve(nb_entities);
        curType = EDGE;
      }
      else if (entity == "Triangles\n")
      {
        connectT3.reserve(nb_entities);
        curType = TRI;
      }
      else if (entity == "Quadrilaterals\n")
      {
        connectQ4.reserve(nb_entities);
        curType = QUAD;
      }
      else if (entity == "Tetrahedra\n")
      {
        connectTH4.reserve(nb_entities);
        curType = TET;
      }
      else if (entity == "Hexahedra\n")
      {
        connectHX8.reserve(nb_entities);
        curType = HEX;
      }
      continue;
    }
  }

  return 0;
}
  static int write(const char* filename, const CoordArray& crd, const IntArr& cnt, const char* elt_type = nullptr, const std::vector<bool>* keep = 0, const std::vector<int>* colors = 0)
  {
    if (crd.size() == 0)
      return 0;
    FILE * file = fopen(filename, "w");
    int  nb_pts(crd.size()), COLS(cnt.size()), dim(3);

  // Header
  fprintf(file, "MeshVersionFormatted 1\n");
  fprintf(file, "Dimension %i\n", dim);
       
  // Points
  fprintf(file, "Vertices\n");
  fprintf(file, "%i\n", nb_pts);
  
  const double* pP;

  if (dim == 3)
  {
    for (int i = 0; i < nb_pts; ++i)
    {
      pP = crd.get(i);
	    fprintf(file, "%f %f %f 0\n", *(pP), *(pP+1), *(pP+2));
    }
  }
  else if (dim ==2)
  {
    for (int i = 0; i < nb_pts; ++i)
    {
      pP = crd.get(i);
      fprintf(file, "%f %f 0\n", *(pP), *(pP + 1));
    }
  }
  
  fprintf(file, "\n");

  if (COLS==0)
  {
    fprintf(file, "End\n");
    fclose(file);
  }

  // Connectivity.
  int nb_nods(cnt.stride()), nb_elts(COLS);

  std::string et;
  if (elt_type) et = elt_type;

  if (nb_nods == 2) // Edges
    fprintf(file, "Edges\n");
  else if (nb_nods == 3) // Triangles
    fprintf(file, "Triangles\n");
  else if ((nb_nods == 4) && (et.find("QUAD") != std::string::npos)) // Q4
    fprintf(file, "Quadrilaterals\n");
  else if ((nb_nods == 4) && (et.find("TETRA") != std::string::npos)) // TH4
    fprintf(file, "Tetrahedra\n");
  else if (nb_nods == 6) // Wedges
    fprintf(file, "Hexahedra\n");
  else //Unknonw
    return 1;

  if (keep)
  {
    for (int i = 0; i < COLS; ++i)
    {
      if ((*keep)[i] == false)
        --nb_elts;
    }
  }

  if (nb_elts == 0)
  {
    fprintf(file, "End\n");
    fclose(file);
    return 0;
  }

	fprintf(file, "%i\n", nb_elts);
  
  bool valid;
  const int *pC(nullptr);
  for (int i = 0; i < COLS; ++i)
  {
    valid = true;

    if (keep && (*keep)[i] == false)
      continue;

	  pC = cnt.get(i);
	  for (int k = 0; (k < nb_nods) && valid; ++k)
	    valid = (*(pC + k) < nb_pts) && (*(pC + k) >= 0);

	  if (valid)
	  {
		  if (!colors)
		  {
			  switch (nb_nods)
			  {
			  case 2: fprintf(file, "%i %i 0\n", *(pC)+1, *(pC + 1) + 1); break;
			  case 3: fprintf(file, "%i %i %i 0\n", *(pC)+1, *(pC + 1) + 1, *(pC + 2) + 1); break;
			  case 4: fprintf(file, "%i %i %i %i 0\n", *(pC)+1, *(pC + 1) + 1, *(pC + 2) + 1, *(pC + 3) + 1); break;
			  case 8: fprintf(file, "%i %i %i %i %i %i %i %i 0\n", *pC + 1, *(pC + 1) + 1, *(pC + 2) + 1, *(pC + 2) + 1, *(pC + 3) + 1, *(pC + 4) + 1, *(pC + 5) + 1, *(pC + 5) + 1); break;
			  default:break;
			  }
		  }
		  else
		  {
			  switch (nb_nods)
			  {
			    case 2: fprintf(file, "%i %i %i\n", *(pC)+1, *(pC + 1) + 1, (*colors)[i]); break;
			    case 3: fprintf(file, "%i %i %i %i\n", *(pC)+1, *(pC + 1) + 1, *(pC + 2) + 1, (*colors)[i]); break;
			    case 4: fprintf(file, "%i %i %i %i %i\n", *(pC)+1, *(pC + 1) + 1, *(pC + 2) + 1, *(pC + 3) + 1, (*colors)[i]); break;
			    case 8: fprintf(file, "%i %i %i %i %i %i %i %i %i\n", *pC + 1, *(pC + 1) + 1, *(pC + 2) + 1, *(pC + 2) + 1, *(pC + 3) + 1, *(pC + 4) + 1, *(pC + 5) + 1, *(pC + 5) + 1, (*colors)[i]); break;
			    default:break;
			  }
		  }
	  }
  }
    
  fprintf(file, "End\n");
  fclose(file);
  
  return 0;
  }

  template <typename ELT>
  static int write(const char* filename, const CoordArray& crd, const std::vector<ELT*>& elts)
  {
    if (crd.size() == 0)
      return 0;
    FILE * file = fopen(filename, "w");
    int  nb_pts(crd.size()), NB_ELTS(elts.size()), dim(3);

    // Header
    fprintf(file, "MeshVersionFormatted 1\n");
    fprintf(file, "Dimension %i\n", dim);

    // Points
    fprintf(file, "Vertices\n");
    fprintf(file, "%i\n", nb_pts);

    const double* pP;

    if (dim == 3)
    {
      for (int i = 0; i < nb_pts; ++i)
      {
        pP = crd.get(i);
        fprintf(file, "%f %f %f 0\n", *(pP), *(pP + 1), *(pP + 2));
      }
    }
    else if (dim == 2)
    {
      for (int i = 0; i < nb_pts; ++i)
      {
        pP = crd.get(i);
        fprintf(file, "%f %f 0\n", *(pP), *(pP + 1));
      }
    }

    fprintf(file, "\n");

    if (NB_ELTS == 0)
    {
      fprintf(file, "End\n");
      fclose(file);
    }

    // Connectivity. ONLY DEAL WITH T3 and Q4
    int nb_T3s = 0;
    int nb_Q4s = 0;

    // check if pure or mixed type
    for (size_t i = 0; i < NB_ELTS; ++i) {
      nb_T3s += (elts[i]->nb_nodes() == 3) ? 1 : 0;
      nb_Q4s += (elts[i]->nb_nodes() == 4) ? 1 : 0;
    }

    if (nb_T3s)
    {
      fprintf(file, "Triangles\n");
      fprintf(file, "%i\n", nb_T3s);
      
      bool valid;
      const int *pC(nullptr);
      for (int i = 0; i < NB_ELTS; ++i){
        
        if (elts[i]->nb_nodes() != 3) continue;

        valid = true;
        pC = elts[i]->nodes();
        for (int k = 0; (k < 3) && valid; ++k)
          valid = (*(pC + k) < nb_pts) && (*(pC + k) >= 0);

        if (valid)
          fprintf(file, "%i %i %i 0\n", *(pC)+1, *(pC + 1) + 1, *(pC + 2) + 1);
      }
    }

    if (nb_Q4s)
    {
      fprintf(file, "Quadrilaterals\n");
      fprintf(file, "%i\n", nb_Q4s);

      bool valid;
      const int *pC(nullptr);
      for (int i = 0; i < NB_ELTS; ++i) {

        if (elts[i]->nb_nodes() != 4) continue;

        valid = true;
        pC = elts[i]->nodes();
        for (int k = 0; (k < 4) && valid; ++k)
          valid = (*(pC + k) < nb_pts) && (*(pC + k) >= 0);

        if (valid)
          fprintf(file, "%i %i %i %i 0\n", *(pC)+1, *(pC + 1) + 1, *(pC + 2) + 1, *(pC + 3) + 1);
      }
    }

    fprintf(file, "End\n");
    fclose(file);

    return 0;
  }
  
  ///
  static void
    get_words
    (const std::string& str_line, char delim, char** oWords)
  {
    //oWords.clear();
    oWords[0]=oWords[1]=oWords[2]=oWords[3]=oWords[4]=oWords[5]=oWords[6]=oWords[7]=oWords[8]=oWords[9]=(char*)"";
    
    char* buf = const_cast<char*>(str_line.c_str());
    char* pch = strtok(buf," ");
    int c=0;
    while (pch != NULL)
    {
      //oWords.push_back(pch);
      oWords[c++]=pch;
      pch = strtok (NULL, " ");
    }
  }
  
  #define white_space(c) ((c) == ' ' || (c) == '\t')
#define valid_digit(c) ((c) >= '0' && (c) <= '9')

  static double fast_atof(const char *p)
  {
    int frac;
    double sign, value, scale;

    // Skip leading white space, if any.

    while (white_space(*p)) {
      p += 1;
    }

    // Get sign, if any.

    sign = 1.0;
    if (*p == '-') {
      sign = -1.0;
      p += 1;

    }
    else if (*p == '+') {
      p += 1;
    }

    // Get digits before decimal point or exponent, if any.

    for (value = 0.0; valid_digit(*p); p += 1) {
      value = value * 10.0 + (*p - '0');
    }

    // Get digits after decimal point, if any.

    if (*p == '.') {
      double pow10 = 10.0;
      p += 1;
      while (valid_digit(*p)) {
        value += (*p - '0') / pow10;
        pow10 *= 10.0;
        p += 1;
      }
    }

    // Handle exponent, if any.

    frac = 0;
    scale = 1.0;
    if ((*p == 'e') || (*p == 'E')) {
      unsigned int expon;

      // Get sign of exponent, if any.

      p += 1;
      if (*p == '-') {
        frac = 1;
        p += 1;

      }
      else if (*p == '+') {
        p += 1;
      }

      // Get digits of exponent, if any.

      for (expon = 0; valid_digit(*p); p += 1) {
        expon = expon * 10 + (*p - '0');
      }
      if (expon > 308) expon = 308;

      // Calculate scaling factor.

      while (expon >= 50) { scale *= 1E50; expon -= 50; }
      while (expon >= 8) { scale *= 1E8;  expon -= 8; }
      while (expon >   0) { scale *= 10.0; expon -= 1; }
    }

    // Return signed and scaled floating point result.

    return sign * (frac ? (value / scale) : (value * scale));
  }

  static int fast_atoindex(const char *p)
  {
    int value;

    // Get digits before decimal point or exponent, if any.

    for (value = 0; valid_digit(*p); p += 1) {
      value = value * 10 + (*p - '0');
    }
    return value;
  }
  
private:
  medith(void){}
  ~medith(void){}
};


#endif
