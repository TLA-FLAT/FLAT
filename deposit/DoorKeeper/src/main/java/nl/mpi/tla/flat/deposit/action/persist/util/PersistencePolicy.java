package nl.mpi.tla.flat.deposit.action.persist.util;

import java.io.File;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;

/**
 * Class representing a single persistence policy.
 * @author guisil
 */
public class PersistencePolicy {

	private final String property;
	private final String regex;
	private final File target;
	
	
	public PersistencePolicy(String property, String regex, File target) {
		this.property = property;
		this.regex = regex;
		this.target = target;
	}


	public String getProperty() {
		return property;
	}

	public String getRegex() {
		return regex;
	}

	public File getTarget() {
		return target;
	}
	
	
	@Override
	public int hashCode() {
		
		HashCodeBuilder hashCodeB = new HashCodeBuilder()
				.append(property)
				.append(regex)
				.append(target);
		
		return hashCodeB.toHashCode();
	}
	
	@Override
	public boolean equals(Object obj) {
		
		if(this == obj) {
			return true;
		}
		
		if(!(obj instanceof PersistencePolicy)) {
			return false;
		}
		
		PersistencePolicy other = (PersistencePolicy) obj;
		
		EqualsBuilder equalsB = new EqualsBuilder()
				.append(property, other.getProperty())
				.append(regex, other.getRegex())
				.append(target, other.getTarget());
		
		return equalsB.isEquals();
	}
	
	@Override
	public String toString() {
		return "property = " + property + "; regex = " + regex + "; target = " + target;
	}
}
