#pragma once
#include "mfem.hpp"
#include <fstream>
#include <iostream>

using namespace std;
using namespace mfem;

void E_exact(const Vector &x, Vector &E);


void FindPtsGetCommonElements(Mesh & mesh0, Mesh & mesh1, 
                              Array<int> & elems0, Array<int> & elems1);

void GetCommonIndices(const Array<int> & list0, const Array<int> & list1, Array<int> & idx0, Array<int> & idx1);

// Given two FiniteElementSpaces and an ElementMap compute
// the dof map between fes0 and fes1
void GetDofMaps(const FiniteElementSpace &fes0, const FiniteElementSpace &fes1, 
                Array<int> & dofs0, Array<int> & dofs1,  
                const Array<int> * elems0_ = nullptr, const Array<int> * elems1_ = nullptr);

void PartitionFE(const FiniteElementSpace * fes, int nrsubmeshes, double ovlp, 
                 Array<FiniteElementSpace*> & fespaces, 
                 Array<Array<int> * > & ElemMaps,
                 Array<Array<int> * > & DofMaps0,
                 Array<Array<int> * > & DofMaps1,
                 Array<Array<int> * > & OvlpMaps0, 
                 Array<Array<int> * > & OvlpMaps1);

// Get tdofs associated with the given list of elements
void RestrictDofs(FiniteElementSpace &fes, int direction, double ovlp, Array<int> & rdofs);
// direction: -1 left (anti-clockwise)
//             1 right (clockwise)
//             0 both the above
// ovlp     :  given in degrees         
void GetElements(Mesh &mesh, double ovlp, int direction, Array<int> & elems);


void DofMapTests(FiniteElementSpace &fes0, FiniteElementSpace &fes1,
                 const Array<int> & dmap0, const Array<int> & dmap1);                 