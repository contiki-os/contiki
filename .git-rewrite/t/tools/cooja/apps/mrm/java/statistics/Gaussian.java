package statistics;

// Gaussian CDF Taylor approximation
// Code borrowed from http://www.cs.princeton.edu/introcs/21function/Gaussian.java.html 19/9 2006

/*************************************************************************
*  Compilation:  javac Gaussian.java
*  Execution:    java Gaussian x mu sigma
*
*  Function to compute the Gaussian pdf (probability density function)
*  and the Gaussian cdf (cumulative density function)
*
*  % java Gaussian 820 1019 209
*  0.17050966869132111
*
*  % java Gaussian 1500 1019 209
*  0.9893164837383883
*
*  % java Gaussian 1500 1025 231
*  0.9801220907365489
*
*************************************************************************/

public class Gaussian {

 // return phi(x) = standard Gaussian pdf
 public static double phi(double x) {
     return Math.exp(-x*x / 2) / Math.sqrt(2 * Math.PI);
 }

 // return phi(x) = Gaussian pdf with mean mu and stddev sigma
 public static double phi(double x, double mu, double sigma) {
     return phi((x - mu) / sigma) / sigma;
 }

 // return Phi(z) = standard Gaussian cdf using Taylor approximation
 public static double Phi(double z) {
     if (z < -8.0) return 0.0;
     if (z >  8.0) return 1.0;
     double sum = 0.0, term = z;
     for (int i = 3; sum + term != sum; i += 2) {
         sum  = sum + term;
         term = term * z * z / i;
     }
     return 0.5 + sum * phi(z);
 }



 // return Phi(z, mu, sigma) = Gaussian cdf with mean mu and stddev sigma
 public static double Phi(double z, double mu, double sigma) {
     return Phi((z - mu) / sigma);
 } 


 public static void main(String[] args) {
     double z     = Double.parseDouble(args[0]);
     double mu    = Double.parseDouble(args[1]);
     double sigma = Double.parseDouble(args[2]);
     System.out.println(Phi(z, mu, sigma));
 }

}
