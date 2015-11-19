package nl.mpi.tla.flat.deposit.action.fits.util;

import edu.harvard.hul.ois.fits.Fits;
import edu.harvard.hul.ois.fits.exceptions.FitsException;

public class FitsFactory {

	//avoid instantiation of the class
	private FitsFactory() {
		throw new AssertionError();
	}
	
	public static Fits getNewFits(String fitsHome) {
		
		try {
			return new Fits(fitsHome);
		} catch (FitsException e) {
			
			// TODO Handle the exception
			return null;
		}
	}
}
