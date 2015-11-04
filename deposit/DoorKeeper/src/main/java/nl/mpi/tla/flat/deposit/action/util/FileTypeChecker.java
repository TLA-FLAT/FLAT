package nl.mpi.tla.flat.deposit.action.util;

/**
 * Class that has access to the list of accepted file types.
 * @author guisil
 */
public class FileTypeChecker {

	//to be used only by the factor method
	private FileTypeChecker() {
	}
	
	/**
	 * Factory method
	 */
	public static FileTypeChecker getNewFileTypeChecker() {
		return new FileTypeChecker();
	}
	
	/**
	 * @param mimetype Mimetype to check
	 * @return true if given mimetype is acceptable
	 */
	public boolean isMimetypeInAcceptableList(String mimetype) {
		
		//TODO to be implemented
		return true;
	}
}
