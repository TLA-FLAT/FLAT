package nl.mpi.tla.flat.deposit.action.util;

import java.util.ArrayList;
import java.util.List;

import javax.xml.transform.Source;

import net.sf.saxon.s9api.SaxonApiException;
import net.sf.saxon.s9api.XdmItem;
import net.sf.saxon.s9api.XdmNode;
import net.sf.saxon.s9api.XdmValue;
import nl.mpi.tla.flat.deposit.util.Saxon;

/**
 * Class used to load the file containing the accepted mimetypes.
 * @author guisil
 */
public class MimetypesLoader {

	//to be used only by the factory method
	private MimetypesLoader() {
	}
	
	public static MimetypesLoader getNewMimetypesLoader() {
		return new MimetypesLoader();
	}
	
	
	/**
	 * Loads the mimetypes from the file.
	 * @param mimetypesSource Source of the mimetypes file
	 * @return List of Strings containing the mimetypes
	 * @throws SaxonApiException
	 */
	public List<String> loadMimetypes(Source mimetypesSource) throws SaxonApiException {
		
		XdmNode mimetypeNode = Saxon.buildDocument(mimetypesSource);
		XdmValue mimetypeValues = Saxon.xpath(mimetypeNode, "/mimetypes/mimetype");
		
        List<String> typesList = new ArrayList<>();
        for (XdmItem mimetypeItem : mimetypeValues) {
            String value = Saxon.xpath2string(mimetypeItem,"@value");
            typesList.add(value);
        }
        return typesList;
    }
}
