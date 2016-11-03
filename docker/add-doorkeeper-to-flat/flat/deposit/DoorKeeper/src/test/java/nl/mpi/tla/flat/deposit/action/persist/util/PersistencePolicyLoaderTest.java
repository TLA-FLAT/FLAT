package nl.mpi.tla.flat.deposit.action.persist.util;

import static org.junit.Assert.*;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.xml.transform.Source;
import javax.xml.transform.stream.StreamSource;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import net.sf.saxon.s9api.SaxonApiException;
import net.sf.saxon.s9api.XdmAtomicValue;
import net.sf.saxon.s9api.XdmValue;
import nl.mpi.tla.flat.deposit.sip.CMDI;

/**
 * @author guisil
 */
public class PersistencePolicyLoaderTest {

	@Rule
	public ExpectedException exceptionCheck = ExpectedException.none();
	
	private File resourcesBaseDir = new File("/proper/persist/path/resources");
	
	private PersistencePolicyLoader persistencePolicyLoader;
	
	private CMDI testSIP;
	private Map<String, XdmValue> datasetProperties;
	private String datasetName = "dataset-123456";
	
	@Before
	public void setUp() throws Exception {
		persistencePolicyLoader = new PersistencePolicyLoader(resourcesBaseDir);
		
		testSIP = new CMDI(new File(getClass().getClassLoader().getResource("test_sip/Progressive_Corpus.cmdi").getFile()));
		
		datasetProperties = new HashMap<>();
		datasetProperties.put("dataset_name", new XdmAtomicValue(datasetName));
	}

	@Test
	public void loadPersistencePolicy_Valid() throws SaxonApiException {
		
		List<PersistencePolicy> allPolicies = new ArrayList<>();
		allPolicies.add(new PersistencePolicy("mimetype", "^.+/pdf$", new File(resourcesBaseDir, "pdf")));
		allPolicies.add(new PersistencePolicy("mimetype", "^text/.+$", new File(resourcesBaseDir, "text")));
		PersistencePolicy defaultPolicy = new PersistencePolicy(null, null, new File(resourcesBaseDir, "default"));
		PersistencePolicies expectedPolicies = new PersistencePolicies(allPolicies, defaultPolicy);

		File persistencePolicyFile = new File(getClass().getClassLoader().getResource("test_policies/valid_policy.xml").getFile());
		
		Source persistencePolicySource = new StreamSource(persistencePolicyFile);
		
		PersistencePolicies retrievedPersistencePolicies = persistencePolicyLoader.loadPersistencePolicies(persistencePolicySource, testSIP, datasetName);
		
		assertEquals("Persistence policies different from expected", expectedPolicies, retrievedPersistencePolicies);
	}
	
	@Test
	public void loadPersistencePolicy_Valid_WithXpathTarget() throws SaxonApiException {
		
		String complete_pdf_folder = datasetName + File.separator + "pdf";
		String complete_text_folder = datasetName + File.separator + "text";
		String complete_default_folder = datasetName + File.separator + "default";
		
		List<PersistencePolicy> allPolicies = new ArrayList<>();
		allPolicies.add(new PersistencePolicy("mimetype", "^.+/pdf$", new File(resourcesBaseDir, complete_pdf_folder)));
		allPolicies.add(new PersistencePolicy("mimetype", "^text/.+$", new File(resourcesBaseDir, complete_text_folder)));
		PersistencePolicy defaultPolicy = new PersistencePolicy(null, null, new File(resourcesBaseDir, complete_default_folder));
		PersistencePolicies expectedPolicies = new PersistencePolicies(allPolicies, defaultPolicy);

		File persistencePolicyFile = new File(getClass().getClassLoader().getResource("test_policies/valid_policy_with_xpath_target.xml").getFile());
		
		Source persistencePolicySource = new StreamSource(persistencePolicyFile);
		
		PersistencePolicies retrievedPersistencePolicies = persistencePolicyLoader.loadPersistencePolicies(persistencePolicySource, testSIP, datasetName);
		
		assertEquals("Persistence policies different from expected", expectedPolicies, retrievedPersistencePolicies);
	}
	
	@Test
	public void loadPersistencePolicy_NoDefaultPolicy() throws SaxonApiException {
		
		exceptionCheck.expect(IllegalStateException.class);
		
		File persistencePolicyFile = new File(getClass().getClassLoader().getResource("test_policies/nodefault_policy.xml").getFile());
		
		Source persistencePolicySource = new StreamSource(persistencePolicyFile);
		
		persistencePolicyLoader.loadPersistencePolicies(persistencePolicySource, testSIP, datasetName);
	}
	
	@Test
	public void loadPersistencePolicy_OnlyDefaultPolicy() throws SaxonApiException {

		List<PersistencePolicy> allPolicies = new ArrayList<>();
		PersistencePolicy defaultPolicy = new PersistencePolicy(null, null, new File(resourcesBaseDir, "default"));
		PersistencePolicies expectedPolicies = new PersistencePolicies(allPolicies, defaultPolicy);

		File persistencePolicyFile = new File(getClass().getClassLoader().getResource("test_policies/onlydefault_policy.xml").getFile());
		
		Source persistencePolicySource = new StreamSource(persistencePolicyFile);
		
		PersistencePolicies retrievedPersistencePolicies = persistencePolicyLoader.loadPersistencePolicies(persistencePolicySource, testSIP, datasetName);
		
		assertEquals("Persistence policies different from expected", expectedPolicies, retrievedPersistencePolicies);
	}
	
	@Test
	public void loadPersistencePolicy_MoreThanOneDefaultPolicy() throws SaxonApiException {

		exceptionCheck.expect(IllegalStateException.class);
		
		File persistencePolicyFile = new File(getClass().getClassLoader().getResource("test_policies/severaldefaults_policy.xml").getFile());
		
		Source persistencePolicySource = new StreamSource(persistencePolicyFile);
		
		persistencePolicyLoader.loadPersistencePolicies(persistencePolicySource, testSIP, datasetName);
	}
	
	@Test
	public void loadPersistencePolicy_invalidPolicyFile() throws SaxonApiException {
		
		exceptionCheck.expect(SaxonApiException.class);
		
		File persistencePolicyFile = new File(getClass().getClassLoader().getResource("test_policies/invalid_policy.xml").getFile());
		
		Source persistencePolicySource = new StreamSource(persistencePolicyFile);
		
		persistencePolicyLoader.loadPersistencePolicies(persistencePolicySource, testSIP, datasetName);
	}
}
