package nl.mpi.tla.flat.deposit.action.fits.util;

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
	public static FITSHandler getNewFITSHandler(String fitsHome, String mimetypesFileLocation) {
		return new FITSHandler(FitsFactory.getNewFits(fitsHome), FileTypeChecker.getNewFileTypeChecker(new File(mimetypesFileLocation)));
	}
	

	/**
	 * Performs the type checking by invoking the appropriate
	 * call within FITS.
	 * @param fileToCheck File to be checked
	 * @return FitsOutput object containing the result of the check
	 * @throws FitsException when something goes wrong with the check
	 */
	public FitsOutput performFitsCheck(File fileToCheck) throws FitsException {
		
		logger.debug("Performing FITS typecheck for file {}", fileToCheck);

		return fits.examine(fileToCheck);
	}
	
	/**
	 * Retrieves the mimetype from within the given FitsOutput object
	 * @param checkedFile File to which the FitsOutput refers to
	 * @param typecheckResult FitsOutput object containing the result of the check
	 * @return String containing the mimetype
	 */
	public String getResultMimetype(File checkedFile, FitsOutput typecheckResult) {
		
		if(typecheckResult == null) {
			return "";
		}
		
		List<FitsIdentity> fileIdentities = typecheckResult.getIdentities();
		
		logger.debug("FITS has detected {} identities for file {}", fileIdentities.size(), checkedFile);
		
		if(fileIdentities.isEmpty()) {
			//TODO no identity found - throw error?
			logger.error("No identity found for file '{}'{}", checkedFile);
			return "";
		}
		
		if(fileIdentities.size() > 1) {
			//TODO the FITS tools didn't agree with each other - throw error?
			logger.error("More than one identity for file '{}'", checkedFile);
			return "";
		}
		
		FitsIdentity identity = fileIdentities.get(0);
		return identity.getMimetype();
	}
	
	/**
	 * Checks if the given mimetype is acceptable.
	 * @param mimetypeToCheck mimetype to check
	 * @return true if the mimetype is acceptable
	 */
	public boolean isMimetypeAcceptable(String mimetypeToCheck) {
		
		logger.debug("Checking if mimetype '{}' is acceptable", mimetypeToCheck);
		
		return fileTypeChecker.isMimetypeInAcceptableList(mimetypeToCheck);
	}
}
