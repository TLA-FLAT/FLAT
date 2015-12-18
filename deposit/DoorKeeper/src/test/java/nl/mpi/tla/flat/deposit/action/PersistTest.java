package nl.mpi.tla.flat.deposit.action;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import static org.mockito.Matchers.any;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.when;

import java.io.File;
import java.net.URI;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.UUID;

import javax.xml.transform.stream.StreamSource;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import com.nitorcreations.junit.runners.NestedRunner;

import net.sf.saxon.s9api.SaxonApiException;
import net.sf.saxon.s9api.XdmAtomicValue;
import net.sf.saxon.s9api.XdmValue;
import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.Resource;
import nl.mpi.tla.flat.deposit.SIP;
import nl.mpi.tla.flat.deposit.action.persist.util.PersistencePolicies;
import nl.mpi.tla.flat.deposit.action.persist.util.PersistencePolicy;
import nl.mpi.tla.flat.deposit.action.persist.util.PersistencePolicyLoader;
import nl.mpi.tla.flat.deposit.action.persist.util.PersistencePolicyMatcher;

/**
 * @author guisil
 */
@RunWith(NestedRunner.class)
public class PersistTest {
	
	@Mock Context mockContext;
	@Mock SIP mockSIP;
	@Mock PersistencePolicyLoader mockPolicyLoader;
	@Mock PersistencePolicyMatcher mockPolicyMatcher;
	
	private Set<Resource> sipResources;
	
	private String resource1_filename = "resource1.pdf";
	private Resource resource1 = new Resource(URI.create("http://some/location/" + resource1_filename), null);
	private File resource1_file = new File("/some/path/" + resource1_filename);
	
	private String resource2_filename = "resource2.xxx";
	private Resource resource2 = new Resource(URI.create("http://some/location/" + resource2_filename), null);
	private File resource2_file = new File("/some/path/" + resource2_filename);
	
	private String resource3_filename = "resource3.txt";
	private Resource resource3 = new Resource(URI.create("http://some/location/" + resource3_filename), null);
	private File resource3_file = new File("/some/path/" + resource3_filename);
	
	private String resourcesBaseDirStr = "/proper/persist/path/resources";
	private File resourcesBaseDir = new File(resourcesBaseDirStr);
	private String policyTarget_pdf = "pdf";
	private String policyTarget_text = "text";
	private String policyTarget_default = "default";
	private PersistencePolicies policies;
	private PersistencePolicy policy1 = new PersistencePolicy("mimetype", "^.+/pdf$", new File(resourcesBaseDir, policyTarget_pdf));
	private PersistencePolicy policy2 = new PersistencePolicy("mimetype", "text/.+$", new File(resourcesBaseDir, policyTarget_text));
	private PersistencePolicy defaultPolicy = new PersistencePolicy(null, null, new File(resourcesBaseDir, policyTarget_default));
	
	
	private Persist persist;
	
	
	@Before
	public void setUp() throws Exception {
		
		sipResources = new HashSet<>();
		
		String resource1_id = UUID.randomUUID().toString();
		resource1.setPID(URI.create("hdl:12345/" + resource1_id));
		resource1.setFID(URI.create("lat:" + resource1_id));
		resource1.setFile(resource1_file);
		resource1.setMime("application/pdf");
		
		String resource2_id = UUID.randomUUID().toString();
		resource2.setPID(URI.create("hdl:12345/" + resource2_id));
		resource2.setFID(URI.create("lat:" + resource2_id));
		resource2.setFile(resource2_file);
		resource2.setMime("unknown");
		
		String resource3_id = UUID.randomUUID().toString();
		resource3.setPID(URI.create("hdl:12345/" + resource3_id));
		resource3.setFID(URI.create("lat:" + resource3_id));
		resource3.setFile(resource3_file);
		resource3.setMime("text/plain");
		
		List<PersistencePolicy> policyList = new ArrayList<>();
		policyList.add(policy1);
		policyList.add(policy2);
		policies = new PersistencePolicies(policyList, defaultPolicy);
		
		Map<String, XdmValue> parameters = new HashMap<>();
		parameters.put("resourcesDir", new XdmAtomicValue(resourcesBaseDirStr));
		parameters.put("policyFile", new XdmAtomicValue("/some/location/resources/policies/persistence-policy.xml"));
		
		MockitoAnnotations.initMocks(this);
		
		persist = spy(new Persist());
		persist.setParameters(parameters);
		
		doReturn(mockPolicyLoader).when(persist).newPersistencePolicyLoader(resourcesBaseDir);
		when(mockContext.getSIP()).thenReturn(mockSIP);
	}
	
	public class UseMimetypeProperty {
		
		public class WhenSipContainsOneResource {
			
			public class WhenMimetypeFoundInPolicy {
				
				@Before
				public void setUp() throws Exception {
					sipResources.add(resource1);
					
					when(mockSIP.getResources()).thenReturn(sipResources);
					when(mockPolicyLoader.loadPersistencePolicies(any(StreamSource.class))).thenReturn(policies);
				}
				
				@Test
				public void resultShouldBeTrue() throws DepositException, SaxonApiException {
					boolean result = persist.perform(mockContext);
					assertTrue("Result should be true", result);
				}
				
				@Test
				public void resourceFileShouldBeSetToMatchedMimetypeLocation() throws DepositException, SaxonApiException {
					File resource1_expected_file = new File(resourcesBaseDirStr + File.separator + policyTarget_pdf + File.separator + resource1_filename);
					persist.perform(mockContext);
					assertEquals("Final resource (1) File different from expected", resource1_expected_file, resource1.getFile());
				}
			}
			
			public class WhenMimetypeNotFoundInPolicy {
				
				@Before
				public void setUp() throws Exception {
					sipResources.add(resource2);
					
					when(mockSIP.getResources()).thenReturn(sipResources);
					when(mockPolicyLoader.loadPersistencePolicies(any(StreamSource.class))).thenReturn(policies);
				}
				
				@Test
				public void resultShouldBeTrue() throws DepositException, SaxonApiException {
					boolean result = persist.perform(mockContext);
					assertTrue("Result should be true", result);
				}
				
				@Test
				public void resourceFileShouldBeSetToDefaultLocation() throws DepositException, SaxonApiException {
					File resource2_expected_file = new File(resourcesBaseDirStr + File.separator + policyTarget_default + File.separator + resource2_filename);
					persist.perform(mockContext);
					assertEquals("Final resource (2) file different from expected", resource2_expected_file, resource2.getFile());
				}
			}
			
			public class WhenPolicyCannotBeLoaded {
				
				@Rule
				public ExpectedException exceptionCheck = ExpectedException.none();
				
				@Test
				public void shouldThrowDepositException() throws DepositException, SaxonApiException {
					
					exceptionCheck.expect(DepositException.class);
					SaxonApiException exceptionToThrow = new SaxonApiException("some issue");
					
					sipResources.add(resource1);
					
					when(mockSIP.getResources()).thenReturn(sipResources);
					when(mockPolicyLoader.loadPersistencePolicies(any(StreamSource.class))).thenThrow(exceptionToThrow);
					
					persist.perform(mockContext);
				}
			}
			
			public class WhenPolicyFileIsInvalid {
				
				@Rule
				public ExpectedException exceptionCheck = ExpectedException.none();
				
				@Test
				public void shouldThrowDepositException() throws DepositException, SaxonApiException {
					
					exceptionCheck.expect(DepositException.class);
					IllegalStateException exceptionToThrow = new IllegalStateException("some issue with the policy file");
					
					sipResources.add(resource1);
					
					when(mockSIP.getResources()).thenReturn(sipResources);
					when(mockPolicyLoader.loadPersistencePolicies(any(StreamSource.class))).thenThrow(exceptionToThrow);
					
					persist.perform(mockContext);
				}
			}
		}
		
		public class WhenSipContainsMultipleResources {
			
			public class WhenAllMimetypesFoundInPolicy {
				
				@Before
				public void setUp() throws Exception {
					sipResources.add(resource1);
					sipResources.add(resource3);
					
					when(mockSIP.getResources()).thenReturn(sipResources);
					when(mockPolicyLoader.loadPersistencePolicies(any(StreamSource.class))).thenReturn(policies);
				}
				
				@Test
				public void resultShouldBeTrue() throws DepositException, SaxonApiException {
					boolean result = persist.perform(mockContext);
					assertTrue("Result should be true", result);
				}
				
				@Test
				public void resourceFilesShouldBeSetToMatchedMimetypeLocations() throws DepositException, SaxonApiException {
					File resource1_expected_file = new File(resourcesBaseDirStr + File.separator + policyTarget_pdf + File.separator + resource1_filename);
					File resource3_expected_file = new File(resourcesBaseDirStr + File.separator + policyTarget_text + File.separator + resource3_filename);
					persist.perform(mockContext);
					assertEquals("Final resource (1) File different from expected", resource1_expected_file, resource1.getFile());
					assertEquals("Final resource (3) File different from expected", resource3_expected_file, resource3.getFile());
				}
			}
			
			public class WhenNotAllMimetypesFoundInPolicy {
				
				@Before
				public void setUp() throws Exception {
					sipResources.add(resource1);
					sipResources.add(resource2);
					
					when(mockSIP.getResources()).thenReturn(sipResources);
					when(mockPolicyLoader.loadPersistencePolicies(any(StreamSource.class))).thenReturn(policies);
				}
				
				@Test
				public void resultShouldBeTrue() throws DepositException, SaxonApiException {
					boolean result = persist.perform(mockContext);
					assertTrue("Result should be true", result);
				}
				
				@Test
				public void resourceFilesShouldBeSetToMatchedMimetypeLocationsOrDefaultLocation() throws DepositException, SaxonApiException {
					File resource1_expected_file = new File(resourcesBaseDirStr + File.separator + policyTarget_pdf + File.separator + resource1_filename);
					File resource2_expected_file = new File(resourcesBaseDirStr + File.separator + policyTarget_default + File.separator + resource2_filename);
					persist.perform(mockContext);
					assertEquals("Final resource (1) File different from expected", resource1_expected_file, resource1.getFile());
					assertEquals("Final resource (2) file different from expected", resource2_expected_file, resource2.getFile());
				}
			}
		}
	}
}
