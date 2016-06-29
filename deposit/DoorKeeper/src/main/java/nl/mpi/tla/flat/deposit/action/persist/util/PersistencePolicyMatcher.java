package nl.mpi.tla.flat.deposit.action.persist.util;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.Resource;

/**
 * Class used to match resources with persistence policies.
 * @author guisil
 */
public class PersistencePolicyMatcher {

	private static final Logger logger = LoggerFactory.getLogger(PersistencePolicyMatcher.class);
	
	private PersistencePolicies policies;
	
	
	public PersistencePolicyMatcher(PersistencePolicies policies) {
		this.policies = policies;
	}
	
	
	/**
	 * Retrieves a persistence policy that matches the given resource.
	 * @param resource Resource to match
	 * @return Appropriate policy
	 * @throws DepositException
	 */
	public PersistencePolicy matchPersistencePolicy(Resource resource) throws DepositException {
		
		logger.info("Trying to find policy for resource '{}'", resource.getFile().getName());
		for(PersistencePolicy policy : policies.getAllPolicies()) {
			if("mimetype".equals(policy.getProperty())) {
				Pattern p = Pattern.compile(policy.getRegex());
				Matcher m = p.matcher(resource.getMime());
				if(m.matches()) {
					logger.info("Found matching policy for mimetype '{}'", resource.getMime());
					return policy;
				}
			} else {
				throw new UnsupportedOperationException("Only mimetype matching supported at the moment for the persistence policy");
			}
		}
		logger.info("A matching policy was not found; using default policy");
		return policies.getDefaultPolicy();
	}
}
