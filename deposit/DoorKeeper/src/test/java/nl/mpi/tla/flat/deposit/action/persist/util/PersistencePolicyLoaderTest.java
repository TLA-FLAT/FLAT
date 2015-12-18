package nl.mpi.tla.flat.deposit.action.persist.util;

import static org.junit.Assert.*;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import javax.xml.transform.Source;
import javax.xml.transform.stream.StreamSource;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import net.sf.saxon.s9api.SaxonApiException;

/**
 * @author guisil
 */
public class PersistencePolicyLoaderTest {

	@Rule
	public ExpectedException exceptionCheck = ExpectedException.none();
	
	private File resourcesBaseDir = new File("/proper/persist/path/resources");
	
	private PersistencePolicyLoader persistencePolicyLoader;
	
	@Before
	public void setUp() throws Exception {
		persistencePolicyLoader = new PersistencePolicyLoader(resourcesBaseDir);
	}

	@Test
	public void loadPersistencePolicy_success() throws SaxonApiException {
		
		List<PersistencePolicy> allPolicies = new ArrayList<>();
		allPolicies.add(new PersistencePolicy("mimetype", "^.+/pdf$", new File(resourcesBaseDir, "pdf")));
		allPolicies.add(new PersistencePolicy("mimetype", "^text/.+$", new File(resourcesBaseDir, "text")));
		PersistencePolicy defaultPolicy = new PersistencePolicy(null, null, new File(resourcesBaseDir, "default"));
		PersistencePolicies expectedPolicies = new PersistencePolicies(allPolicies, defaultPolicy);

		File persistencePolicyFile = new File(getClass().getClassLoader().getResource("test_policies/valid_policy.xml").getFile());
		
		Source persistencePolicySource = new StreamSource(persistencePolicyFile);
		
		PersistencePolicies retrievedPersistencePolicies = persistencePolicyLoader.loadPersistencePolicies(persistencePolicySource);
		
		assertEquals("Persistence policies different from expected", expectedPolicies, retrievedPersistencePolicies);
	}
	
	@Test
	public void loadPersistencePolicy_NoDefaultPolicy() throws SaxonApiException {
		
		exceptionCheck.expect(IllegalStateException.class);
		
		File persistencePolicyFile = new File(getClass().getClassLoader().getResource("test_policies/nodefault_policy.xml").getFile());
		
		Source persistencePolicySource = new StreamSource(persistencePolicyFile);
		
		persistencePolicyLoader.loadPersistencePolicies(persistencePolicySource);
	}
	
	@Test
	public void loadPersistencePolicy_OnlyDefaultPolicy() throws SaxonApiException {

		List<PersistencePolicy> allPolicies = new ArrayList<>();
		PersistencePolicy defaultPolicy = new PersistencePolicy(null, null, new File(resourcesBaseDir, "default"));
		PersistencePolicies expectedPolicies = new PersistencePolicies(allPolicies, defaultPolicy);

		File persistencePolicyFile = new File(getClass().getClassLoader().getResource("test_policies/onlydefault_policy.xml").getFile());
		
		Source persistencePolicySource = new StreamSource(persistencePolicyFile);
		
		PersistencePolicies retrievedPersistencePolicies = persistencePolicyLoader.loadPersistencePolicies(persistencePolicySource);
		
		assertEquals("Persistence policies different from expected", expectedPolicies, retrievedPersistencePolicies);
	}
	
	@Test
	public void loadPersistencePolicy_MoreThanOneDefaultPolicy() throws SaxonApiException {

		exceptionCheck.expect(IllegalStateException.class);
		
		File persistencePolicyFile = new File(getClass().getClassLoader().getResource("test_policies/severaldefaults_policy.xml").getFile());
		
		Source persistencePolicySource = new StreamSource(persistencePolicyFile);
		
		persistencePolicyLoader.loadPersistencePolicies(persistencePolicySource);
	}
	
	@Test
	public void loadPersistencePolicy_invalidPolicyFile() throws SaxonApiException {
		
		exceptionCheck.expect(SaxonApiException.class);
		
		File persistencePolicyFile = new File(getClass().getClassLoader().getResource("test_policies/invalid_policy.xml").getFile());
		
		Source persistencePolicySource = new StreamSource(persistencePolicyFile);
		
		persistencePolicyLoader.loadPersistencePolicies(persistencePolicySource);
	}
}
