/***************************************************************************
 *   Copyright (C) 2011 by Jan Wagner                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
//===========================================================================
// SVN properties (DO NOT CHANGE)
//
// $Id: $
// $HeadURL: $
// $LastChangedRevision: $
// $Author: $
// $LastChangedDate: $
//
//============================================================================

#include "Analyzer.h"

#include <armadillo>

/**
 * Allocate output matrices or output cubes.
 * @param[in]  Nant    Dimension of 2D square matrix.
 * @param[in]  Nchan   Number of 2D slices in 3D cube.
 * @param[in]  NdecoM  Number of matrices to store decomposition (1 for Eig, 2 for QR, 2 for SVD, etc)
 * @param[in]  NdecoV  Number of vectors to store decomposition (1 for Eig, 0 for QR, 1 for SVD, etc)
 * If Nchan<=1, only the _single_out_matrices[] is allocated.
 * Otherwise, only the _batch_out_cubes[] is allocated.
 */
void Decomposition::cstor_alloc(const int Nant, const int Nchan, const int NdecoM, const int NdecoV)
{

   // Matrice(s) with eigenvectors from decomposition
   for (int decoMat=0; (decoMat<NdecoM) && (decoMat<3); decoMat++) {
      if (Nchan <= 1) {
         _single_out_matrices[decoMat] = arma::zeros<arma::Mat<arma::cx_double> >(Nant,Nant);
      } else {   
         _batch_out_matrices[decoMat] = arma::zeros<arma::Cube<arma::cx_double> >(Nant,Nant, Nchan);
      }
   }

   // Vector(s) with the eigenvalues from diagonal of decomposition
   if (NdecoV >= 1) {
      if (Nchan <= 1) {
         _single_out_vector = arma::zeros<arma::Col<double> >(Nant);
      } else {
         _batch_out_vectors = arma::zeros<arma::Mat<double> >(Nant, Nchan);
      }
   }
}

/**
 * Perform batch decomposition of all covariances in the argument class.
 * @param[in]  allRxx  The covariance class containing one or more matrices.
 * @return             Zero on success.
 */
int Decomposition::decompose(Covariance& cov) {
   const arma::Cube<arma::cx_double>& allRxx = cov.get();
   arma::Mat<arma::cx_double> Rxx;

   for (unsigned int chan=0; chan<allRxx.n_slices; chan++) {
      Rxx = allRxx.slice(chan);
      this->do_decomposition(chan, Rxx);
   }

   return 0;
}


/**
 * Decompose covariance matrix and store results into output array
 * specified by index 'sliceNr'.
 * Dummy template function only.
 * @param[in]  sliceNr   Index into output cube (0=single matrix, 1..N+1=cube storage)
 * @param[in]  Rxx       Matrix to decompose
 * @return 0 on success
 */
int Decomposition::do_decomposition(int sliceNr, arma::Mat<arma::cx_double>& Rxx)
{
   return 0;
}
