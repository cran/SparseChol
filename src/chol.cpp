#include "SparseChol.h"
using namespace Rcpp;

//' Sparse Cholesky decomposition with sparse representation
//' 
//' Sparse Cholesky decomposition with sparse representation
//' 
//' @details
//' Generates the LDL decomposition of a symmetric, sparse matrix using the method 
//' described by Timothy Davis (see references). Required input is a matrix
//' in sparse format from the matrix package, see \link[Matrix]{sparseMatrix}, or the
//' package function \link[SparseChol]{dense_to_sparse}. To instead use a matrix directly, see \link[SparseChol]{sparse_chol}.
//' @param n Integer specifying the dimension of the matrix
//' @param Ai Integer vector specifying the row positions of the non-zero values of the matrix
//' @param Ap numeric (integer valued) vector of pointers, one for each column (or row), to the initial (zero-based) index of elements in the column (or row).
//' @param Ax values of the non-zero matrix entries
//' @return A list with elements n, Ai, Ap, Ax (corresponding to above arguments) for matrix L, and element D, which 
//' contains the diagonal values of matrix D.
//' @examples
//' n <- 10
//' Ap <- c(0, 1, 2, 3, 4, 6, 7, 9, 11, 15, 19)
//' Ai <- c(1, 2, 3, 4, 2,5, 6, 5,7, 5,8, 1,5,8,9, 2,5,7,10)
//' Ax <- c(1.7, 1., 1.5, 1.1, .02,2.6, 1.2, .16,1.3, .09,1.6,
//'           .13,.52,.11,1.4, .01,.53,.56,3.1)
//' out <-sparse_chol_crs(n,Ap,Ai,Ax)
//' sparse_L(out)
//' sparse_D(out)
// [[Rcpp::export]]
Rcpp::List sparse_chol_crs(int n,
                          std::vector<int> Ap,
                          std::vector<int> Ai,
                          std::vector<double> Ax){
  sparse mat(Ap);
  mat.n = n;
  mat.m = n;
  mat.Ai = Ai;
  if(Ai[0] != 0)std::for_each(mat.Ai.begin(), mat.Ai.end(), [](int &n){ n--; });
  if(Ap[0] != 0)std::for_each(mat.Ap.begin(), mat.Ap.end(), [](int &n){ n--; });
  mat.Ax = Ax;
  
  SparseChol chol(mat);
  int d = chol.ldl_numeric();
  Rcpp::Rcout << "d: " << d;
  // if(Ai[0] != 0)std::for_each(chol.L.Ai.begin(), chol.L.Ai.end(), [](int &n){ n++; });
  // if(Ap[0] != 0)std::for_each(chol.L.Ap.begin(), chol.L.Ap.end(), [](int &n){ n++; });
  return Rcpp::List::create(_["n"] = chol.L.n,_["Ap"] = chol.L.Ap,
                            _["Ai"] = chol.L.Ai,_["Ax"] = chol.L.Ax,
                            _["D"] = chol.D);
}

//' Sparse Cholesky decomposition
//' 
//' Sparse Cholesky decomposition
//' 
//' @details
//' Generates the LDL decomposition of a symmetric, sparse matrix using the method 
//' described by Timothy Davis (see references). This function accepts a standard matrix,
//' converts to sparse format, generates the LDL decomposition and returns the Cholesky 
//' decomposition LD^0.5.
//' @param mat A matrix
//' @return A lower-triangular matrix.
//' @examples
//' M <- diag(10)
//' #put a few random values in
//' M[lower.tri(M)][seq(1,45,by=5)] <- c(0.1,0.5,0.9,0.6,0.8,0.9,0.2,0.3,0.1)
//' M[upper.tri(M)][seq(1,45,by=5)] <- c(0.1,0.5,0.9,0.6,0.8,0.9,0.2,0.3,0.1)
//' L <- sparse_chol(M)
// [[Rcpp::export]]
SEXP sparse_chol(Rcpp::NumericMatrix mat){
  int n = mat.rows();
  int m = mat.cols();
  if(m!=n)Rcpp::stop("Matrix must be square");
  sparse A(n,m,mat);
  SparseChol chol(A);
  int d = chol.ldl_numeric();
  sparse B = chol.LD();
  NumericVector output = wrap(B.dense(false));
  output.attr("dim") = Rcpp::Dimension(n, m);
  return output;
}

//' AMD ordering
//'
//' AMD ordering
//'
//' @details
//' Generates the approximate minimum degree ordering of the matrix for use in efficient
//' Cholesky decomposition of PAP^T.
//' @param mat A matrix
//' @return A list with the permutation vector and it's inverse.
// [[Rcpp::export]]
SEXP amd_order(Rcpp::NumericMatrix mat){
   int n = mat.rows();
   int m = mat.cols();
   if(m!=n)Rcpp::stop("Matrix must be square");
   sparse A(n,m,mat);
   intvec P = A.permute();
   intvec Pinv = A.permute_inv();
   return wrap(Rcpp::List::create(
     _["P"] = P,
     _["Pinv"] = Pinv
   ));
}

//' Generate sparse matrix representation of a matrix
//' 
//' Generate sparse matrix representation of a matrix
//' 
//' @param mat A matrix
//' @return A list with the matrix in compressed row storage format.
//' @examples
//' M <- diag(10)
//' #put a few random values in
//' M[lower.tri(M)][seq(1,45,by=5)] <- c(0.1,0.5,0.9,0.6,0.8,0.9,0.2,0.3,0.1)
//' M[upper.tri(M)][seq(1,45,by=5)] <- c(0.1,0.5,0.9,0.6,0.8,0.9,0.2,0.3,0.1)
//' L <- dense_to_sparse(M)
// [[Rcpp::export]]
Rcpp::List dense_to_sparse(Rcpp::NumericMatrix mat){
  int n = mat.rows();
  int m = mat.cols();
  if(m!=n)Rcpp::stop("Matrix must be square");
  sparse A(n,m,mat);
  return Rcpp::List::create(_["n"] = A.n,_["Ap"] = A.Ap,
                            _["Ai"] = A.Ai,_["Ax"] = A.Ax);
}
