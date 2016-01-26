package nl.mpi.tla.flat.deposit.action.persist.util;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Document;

import net.sf.saxon.s9api.SaxonApiException;
import net.sf.saxon.s9api.XdmNode;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.util.Saxon;

/**
 * Class used to retrieve the name to use for the dataset base folder.
 * @author guisil
 */
public class PersistDatasetNameRetriever {
	
	private static Logger logger = LoggerFactory.getLogger(PersistDatasetNameRetriever.class);

	/**
	 * Gets the dataset name by using the given xpath expression on the given SIP record.
	 * @param sipRecord SIP record to get the value from
	 * @param datasetNameXpath XPATH expression to use 
	 * @return name to use as base for the dataset folder
	 */
	public String getDatasetName(Document sipRecord, String datasetNameXpath) throws DepositException {

    	XdmNode sipNode = Saxon.wrapNode(sipRecord);
    	String datasetName;
    	try {
			datasetName = Saxon.xpath2string(sipNode, datasetNameXpath);
		} catch (SaxonApiException ex) {
			String message = "Error extracting name to use as base folder for the resource policy";
			logger.error(message, ex);
			throw new DepositException(message, ex);
		}
    	
    	return datasetName;
	}
}
