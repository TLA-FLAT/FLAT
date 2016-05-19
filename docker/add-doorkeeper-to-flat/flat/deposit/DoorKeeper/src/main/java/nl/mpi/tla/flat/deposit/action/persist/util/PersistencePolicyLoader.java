package nl.mpi.tla.flat.deposit.action.persist.util;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.xml.transform.Source;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import net.sf.saxon.s9api.SaxonApiException;
import net.sf.saxon.s9api.XdmAtomicValue;
import net.sf.saxon.s9api.XdmItem;
import net.sf.saxon.s9api.XdmNode;
import net.sf.saxon.s9api.XdmValue;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.SIP;
import nl.mpi.tla.flat.deposit.util.Saxon;

/**
 * Class used to load the persistence policies.
 * @author guisil
 */
public class PersistencePolicyLoader {

	private static final Logger logger = LoggerFactory.getLogger(PersistencePolicyLoader.class);

	private final File resourcesBaseDir;
	
	
	public PersistencePolicyLoader(File resourcesBaseDir) {
		this.resourcesBaseDir = resourcesBaseDir;
	}
	
	
	/**
	 * Loads the Persistence policies from the file.
	 * @param mimetypesSource Source of the Persist policies file
	 * @param sip SIP being currently processed
	 * @param datasetName name to be used as base for the folders mentioned in the policy
	 * @return List of Strings containing the Persist policies
	 * @throws SaxonApiException
	 * @throws DepositException 
	 */
	public PersistencePolicies loadPersistencePolicies(Source persistencePoliciesSource, SIP sip, String datasetName) throws SaxonApiException {
		
		logger.debug("Loading persistence policy from source {}", persistencePoliciesSource.getSystemId());
		
		XdmNode persistencePolicyNode = Saxon.buildDocument(persistencePoliciesSource);
		XdmValue persistencePolicyValues = Saxon.xpath(persistencePolicyNode, "/persistence-policies/persistence-policy");
		XdmValue persistenceDefaultPolicyValues = Saxon.xpath(persistencePolicyNode, "/persistence-policies/default-persistence-policy");

        if(persistenceDefaultPolicyValues.size() != 1) {
        	throw new IllegalStateException("Should have one default policy");
        }
        
    	XdmNode sipNode = Saxon.wrapNode(sip.getRecord());
        
        String defaultTarget = Saxon.xpath2string(persistenceDefaultPolicyValues.iterator().next(), "@target");
        String defaultActualTarget = getTargetWithResolvedVariable(defaultTarget, sipNode, datasetName);
        PersistencePolicy defaultPolicy = new PersistencePolicy(null, null, new File(resourcesBaseDir, defaultActualTarget));
		
		List<PersistencePolicy> policyList = new ArrayList<>();
        for (XdmItem persistencePolicyItem : persistencePolicyValues) {
        	String property = Saxon.xpath2string(persistencePolicyItem, "@property");
        	String regex = Saxon.xpath2string(persistencePolicyItem, "@regex");
        	String targetWithVariable = Saxon.xpath2string(persistencePolicyItem, "@target");
        	String actualTarget = getTargetWithResolvedVariable(targetWithVariable, sipNode, datasetName);
        	policyList.add(new PersistencePolicy(property, regex, new File(resourcesBaseDir, actualTarget)));
        }
        
        logger.debug("Persistence policy loaded.");
        return new PersistencePolicies(policyList, defaultPolicy);
    }
	
	
	private String getTargetWithResolvedVariable(String targetWithVariable, XdmNode node, String variable) throws SaxonApiException {
		Map<String, XdmValue> datasetProperties = new HashMap<>();
    	datasetProperties.put("dataset_name", new XdmAtomicValue(variable));
    	String actualTarget = Saxon.avt(targetWithVariable, node, datasetProperties);
    	return actualTarget;
	}
}
