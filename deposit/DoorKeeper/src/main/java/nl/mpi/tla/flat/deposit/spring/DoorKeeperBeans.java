package nl.mpi.tla.flat.deposit.spring;

import edu.harvard.hul.ois.fits.Fits;
import edu.harvard.hul.ois.fits.exceptions.FitsException;
import nl.mpi.tla.flat.deposit.action.util.TypeCheckHandler;
import nl.mpi.tla.flat.deposit.action.util.implementation.FITSHandler;
import nl.mpi.tla.flat.deposit.action.util.implementation.FileTypeChecker;

/**
 * Configuration class containing some beans to be used in the application.
 * @author guisil
 */
public class DoorKeeperBeans {

	//TODO Make this a @Configuration class, with @Beans
	
	
	public Fits fits() {
		
		//TODO Change the call and pass the FITS home variable
		try {
			return new Fits();
		} catch (FitsException e) {
			
			// TODO Handle the exception
			return null;
		}
	}
	
	public FileTypeChecker fileTypeChecker() {
		return new FileTypeChecker();
	}
	
	public TypeCheckHandler typeCheckHandler() {
		return new FITSHandler(fits(), fileTypeChecker());
	}
}
