package statistics;

public class GaussianWrapper {

  /**
   * Returns standard Gaussian cdf approximation based on algortihm for error function.
   * 
   * @param value Value
   * @return Probability
   */
  public static double cdfErrorAlgo(double value) {
    return CDF_Normal.normp(value);
  }

  /**
   * Returns Gaussian cdf approximation based on algorithm for error function.
   * 
   * @param value Value
   * @param mean Mean value
   * @param stdDev Standard deviance
   * @return Probability
   */
  public static double cdfErrorAlgo(double value, double mean, double stdDev) {
    return CDF_Normal.normp((value - mean) / stdDev);      
  }
    
  /**
   * Returns standard Gaussian cdf using Taylor approximation.
   * 
   * @param value Value
   * @return Probability
   */
  public static double cdfTaylor(double value) {
    return Gaussian.Phi(value);
  }

  /**
   * Returns Gaussian cdf using Taylor approximation .
   * 
   * @param value Value
   * @param mean Mean value
   * @param stdDev Standard deviance
   * @return Probability
   */
  public static double cdfTaylor(double value, double mean, double stdDev) {
    return Gaussian.Phi(value, mean, stdDev);
  }
  
}

