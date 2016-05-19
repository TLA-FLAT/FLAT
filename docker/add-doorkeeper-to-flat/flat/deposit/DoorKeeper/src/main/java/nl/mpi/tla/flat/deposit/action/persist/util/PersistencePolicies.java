package nl.mpi.tla.flat.deposit.action.persist.util;

import java.util.List;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;

/**
 * Class representing the persistence policices.
 * @author guisil
 */
public class PersistencePolicies {

	private List<PersistencePolicy> allPolicies;
	private PersistencePolicy defaultPolicy;
	
	public PersistencePolicies(List<PersistencePolicy> allPolicies, PersistencePolicy defaultPolicy) {
		this.allPolicies = allPolicies;
		this.defaultPolicy = defaultPolicy;
	}
	
	public List<PersistencePolicy> getAllPolicies() {
		return allPolicies;
	}
	public void setAllPolicies(List<PersistencePolicy> allPolicies) {
		this.allPolicies = allPolicies;
	}
	
	public PersistencePolicy getDefaultPolicy() {
		return defaultPolicy;
	}
	public void setDefaultPolicy(PersistencePolicy defaultPolicy) {
		this.defaultPolicy = defaultPolicy;
	}
	
	
	@Override
	public int hashCode() {
		
		HashCodeBuilder hashCodeB = new HashCodeBuilder()
				.append(allPolicies)
				.append(defaultPolicy);
		
		return hashCodeB.toHashCode();
	}
	
	@Override
	public boolean equals(Object obj) {
		
		if(this == obj) {
			return true;
		}
		
		if(!(obj instanceof PersistencePolicies)) {
			return false;
		}
		
		PersistencePolicies other = (PersistencePolicies) obj;
		
		EqualsBuilder equalsB = new EqualsBuilder()
				.append(allPolicies, other.getAllPolicies())
				.append(defaultPolicy, other.getDefaultPolicy());
		
		return equalsB.isEquals();
	}
	
	@Override
	public String toString() {
		return "all policies: " + allPolicies + "; default policy: " + defaultPolicy;
	}
}
