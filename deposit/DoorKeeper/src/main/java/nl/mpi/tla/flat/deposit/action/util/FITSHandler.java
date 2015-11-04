package nl.mpi.tla.flat.deposit.action.util;

import java.io.File;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.harvard.hul.ois.fits.Fits;
import edu.harvard.hul.ois.fits.FitsOutput;
import edu.harvard.hul.ois.fits.exceptions.FitsException;
import edu.harvard.hul.ois.fits.identity.FitsIdentity;

/**
 * Helper class that interacts with FITS.
 * @author guisil
 */
public class FITSHandler {

	private static final Logger logger = LoggerFactory.getLogger(FITSHandler.class);
	
	private Fits fits;
	private FileTypeChecker fileTypeChecker;
	
	public FITSHandler() {
		
	}

	//to be used only by the factory method
	private FITSHandler(Fits fits, FileTypeChecker fileTypeChecker) {
		this.fits = fits;
		this.fileTypeChecker = fileTypeChecker;
	}
	
	/**
	 * Factory method
	 */
	public static FITSHandler getNewFITSHandler(String fitsHome) {
		return new FITSHandler(FitsFactory.getNewFits(fitsHome), FileTypeChecker.getNewFileTypeChecker());
	}
	
	/**
	 * Checks if the given file is acceptable for deposit.
	 * @param fileToCheck File to check
	 * @return true if the file is acceptable in the archive
	 */
	public boolean isFileAcceptable(File fileToCheck) {

		FitsOutput fitsOutput = null;
		try {
			fitsOutput = fits.examine(fileToCheck);
		} catch (FitsException e) {
			// TODO Auto-generated catch block
			logger.error("ERROR", e);
		}
		
		List<FitsIdentity> fileIdentities = fitsOutput.getIdentities();
		
		if(fileIdentities.isEmpty()) {
			//TODO no identity found - throw error in this case?
			logger.debug("No identity found for file '{}'{}", fileToCheck);
			return false;
		}
		
		if(fileIdentities.size() > 1) {
			//TODO the FITS tools didn't agree with each other - which one to use in this case?
			logger.debug("More than one identity for file '{}'", fileToCheck);
		}
		
		FitsIdentity identity = fileIdentities.get(0);
		
		return fileTypeChecker.isMimetypeInAcceptableList(identity.getMimetype());
	}

}
