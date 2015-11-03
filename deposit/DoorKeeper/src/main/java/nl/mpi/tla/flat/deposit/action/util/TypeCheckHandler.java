package nl.mpi.tla.flat.deposit.action.util;

import java.io.File;

/**
 * Interface for the possible objects responsible
 * for performing the file type checking.
 * @author guisil
 */
public interface TypeCheckHandler {

	/**
	 * Checks if the given file is acceptable for deposit.
	 * @param fileToCheck File to check
	 * @return true if the file is acceptable in the archive
	 */
	public boolean isFileAcceptable(File fileToCheck);
}
