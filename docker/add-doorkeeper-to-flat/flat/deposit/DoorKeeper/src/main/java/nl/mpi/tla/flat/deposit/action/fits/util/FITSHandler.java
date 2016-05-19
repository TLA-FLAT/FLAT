package nl.mpi.tla.flat.deposit.action.fits.util;

import java.io.File;
import java.net.URL;
import java.util.Iterator;
import java.util.List;
import javax.xml.transform.stream.StreamSource;
import net.sf.saxon.s9api.XdmItem;
import net.sf.saxon.s9api.XdmNode;
import net.sf.saxon.s9api.XdmValue;
import static nl.mpi.tla.flat.deposit.util.Global.NAMESPACES;
import nl.mpi.tla.flat.deposit.util.Saxon;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Helper class that interacts with FITS.
 * @author guisil
 */
public class FITSHandler {

    private static final Logger logger = LoggerFactory.getLogger(FITSHandler.class);
	
    private URL serviceEndpoint;
    private FileTypeChecker fileTypeChecker;
	
    public FITSHandler(URL fitsService, String mimetypesFileLocation) {
        this.serviceEndpoint = fitsService;
        this.fileTypeChecker = FileTypeChecker.getNewFileTypeChecker(new File(mimetypesFileLocation));
    }
	
    /**
     * Performs the type checking by invoking the appropriate
     * call within FITS.
     * @param fileToCheck File to be checked
     * @return Document doc containing the result of the check
     * @throws Exception when something goes wrong with the check
     */
    public XdmNode performFitsCheck(File fileToCheck) throws Exception {
		
        logger.debug("Performing FITS typecheck for file {}", fileToCheck);
        
        URL call = new URL(serviceEndpoint,"examine?file="+fileToCheck.getAbsolutePath());
        
        logger.debug("Performing FITS typecheck via {}", call);

        return Saxon.buildDocument(new StreamSource(call.toString()));
    }
	
    /**
     * Retrieves the mimetype from within the given FitsOutput object
     * @param checkedFile File to which the FitsOutput refers to
     * @param typecheckResult FITS doc containing the result of the check
     * @return String containing the mimetype
     */
    public String getResultMimetype(File checkedFile, XdmNode typecheckResult) throws Exception {
		
        if(typecheckResult == null) {
            return "";
        }
		
        XdmValue fileIdentities = Saxon.xpath(typecheckResult,"distinct-values(/fits:fits/fits:identification/fits:identity/@mimetype)",null,NAMESPACES);
    	
        logger.debug("FITS has detected {} mimetypes for file {}", fileIdentities.size(), checkedFile);
		
        if(fileIdentities.size()==0) {
            //TODO no identity found - throw error?
            logger.error("No mimetype found for file '{}'{}", checkedFile);
            return "";
        }
		
        if(fileIdentities.size() > 1) {
            //TODO the FITS tools didn't agree with each other - throw error?
            logger.error("More than one mimetype for file '{}'", checkedFile);
            return "";
        }
		
        return fileIdentities.itemAt(0).getStringValue();
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
