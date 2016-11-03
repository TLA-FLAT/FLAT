package nl.mpi.tla.flat.deposit.action;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.mockito.Matchers.any;
import static org.mockito.Matchers.eq;
import static org.mockito.Matchers.same;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.when;

import java.io.File;
import java.net.URI;
import java.nio.file.Files;
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
import org.junit.rules.TemporaryFolder;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.w3c.dom.Document;

import com.nitorcreations.junit.runners.NestedRunner;

import net.sf.saxon.s9api.SaxonApiException;
import net.sf.saxon.s9api.XdmAtomicValue;
import net.sf.saxon.s9api.XdmValue;
import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.sip.Resource;
import nl.mpi.tla.flat.deposit.sip.SIPInterface;
import nl.mpi.tla.flat.deposit.action.persist.util.PersistDatasetNameRetriever;
import nl.mpi.tla.flat.deposit.action.persist.util.PersistencePolicies;
import nl.mpi.tla.flat.deposit.action.persist.util.PersistencePolicy;
import nl.mpi.tla.flat.deposit.action.persist.util.PersistencePolicyLoader;
import nl.mpi.tla.flat.deposit.action.persist.util.PersistencePolicyMatcher;

/**
 * @author guisil
 */
@RunWith(NestedRunner.class)
public class PersistTest {
	
	@Rule public TemporaryFolder testFolder = new TemporaryFolder();
	
	@Mock Context mockContext;
	@Mock SIPInterface mockSIP;
	@Mock Document mockRecord;
	@Mock PersistencePolicyLoader mockPolicyLoader;
	@Mock PersistencePolicyMatcher mockPolicyMatcher;
	@Mock PersistDatasetNameRetriever mockDatasetNameRetriever;
	
	private Set<Resource> sipResources;
	
	private String resourcesInitialFolderName = "initialFolder";
	private File resourcesInitialDir;
	
	private String resource1_filename = "resource1.pdf";
	private Resource resource1 = new Resource(URI.create("http://some/location/" + resource1_filename), null);
	private File resource1_file;
	
	private String resource2_filename = "resource2.xxx";
	private Resource resource2 = new Resource(URI.create("http://some/location/" + resource2_filename), null);
	private File resource2_file;
	
	private String resource3_filename = "resource3.txt";
	private Resource resource3 = new Resource(URI.create("http://some/location/" + resource3_filename), null);
	private File resource3_file;
	
	private String datasetNameXpath = "replace(//*[name()='MdSelfLink'], 'hdl:{$tlaHandlePrefix}/',''";
	private String datasetName = "dataset-123456";
	
	private File resourcesTargetDir;
	private String policyTarget_pdf = datasetName + File.separator + "pdf";
	private String policyTarget_text = datasetName + File.separator + "text";
	private String policyTarget_default = datasetName + File.separator + "default";
	private PersistencePolicies policies;
	private PersistencePolicy policy1;
	private PersistencePolicy policy2;
	private PersistencePolicy defaultPolicy;
	
	
	private Persist persist;
	
	
	@Before
	public void setUp() throws Exception {
		
		testFolder.create();
		assertTrue("Test folder was not created", testFolder.getRoot().exists());
		
		resourcesInitialDir = testFolder.newFolder(resourcesInitialFolderName);
		assertTrue("Initial folder was not created", resourcesInitialDir.exists());
		resource1_file = new File(resourcesInitialDir, resource1_filename);
		resource1_file.createNewFile();
		assertTrue("resource1_file was not created", resource1_file.exists());
		resource2_file = new File(resourcesInitialDir, resource2_filename);
		resource2_file.createNewFile();
		assertTrue("resource2_file was not created", resource2_file.exists());
		resource3_file = new File(resourcesInitialDir, resource3_filename);
		resource3_file.createNewFile();
		assertTrue("resource3_file was not created", resource3_file.exists());
		
		resourcesTargetDir = testFolder.newFolder(datasetName);
		assertTrue("Target folder was not created", resourcesTargetDir.exists());
		
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
		
		
		policy1 = new PersistencePolicy("mimetype", "^.+/pdf$", new File(resourcesTargetDir, policyTarget_pdf));
		policy2 = new PersistencePolicy("mimetype", "text/.+$", new File(resourcesTargetDir, policyTarget_text));
		defaultPolicy = new PersistencePolicy(null, null, new File(resourcesTargetDir, policyTarget_default));
		
		List<PersistencePolicy> policyList = new ArrayList<>();
		policyList.add(policy1);
		policyList.add(policy2);
		policies = new PersistencePolicies(policyList, defaultPolicy);
		
		Map<String, XdmValue> parameters = new HashMap<>();
		parameters.put("resourcesDir", new XdmAtomicValue(resourcesTargetDir.getAbsolutePath()));
		parameters.put("policyFile", new XdmAtomicValue("/some/location/resources/policies/persistence-policy.xml"));
		parameters.put("xpathDatasetName", new XdmAtomicValue(datasetNameXpath));
		
		MockitoAnnotations.initMocks(this);
		
		persist = spy(new Persist());
		persist.setParameters(parameters);
		
		doReturn(mockPolicyLoader).when(persist).newPersistencePolicyLoader(resourcesTargetDir);
		doReturn(mockDatasetNameRetriever).when(persist).newPersistDatasetNameRetriever();
		when(mockContext.getSIP()).thenReturn(mockSIP);
		when(mockSIP.getRecord()).thenReturn(mockRecord);
		when(mockDatasetNameRetriever.getDatasetName(mockRecord, datasetNameXpath)).thenReturn(datasetName);
	}
	
	
	public class UseMimetypeProperty {
		
		public class WhenSipContainsOneResource {
			
			public class WhenMimetypeFoundInPolicy {
				
				@Before
				public void setUp() throws Exception {
					sipResources.add(resource1);
					
					when(mockSIP.getResources()).thenReturn(sipResources);
					when(mockPolicyLoader.loadPersistencePolicies(any(StreamSource.class), same(mockSIP), eq(datasetName))).thenReturn(policies);
				}
				
				@Test
				public void resultShouldBeTrue() throws DepositException {
					boolean result = persist.perform(mockContext);
					assertTrue("Result should be true", result);
				}
				
				@Test
				public void resourceFileShouldBeSetToMatchedMimetypeLocation() throws DepositException {
					File resource1_expected_file = new File(new File(resourcesTargetDir, policyTarget_pdf), resource1_filename);
					persist.perform(mockContext);
					assertEquals("Final resource (1) File different from expected", resource1_expected_file, resource1.getFile());
				}
				
				@Test
				public void resourceFileShouldBeMovedFromInitialLocationToTargetLocation( ) throws DepositException {
					File resource1_expected_file = new File(new File(resourcesTargetDir, policyTarget_pdf), resource1_filename);
					persist.perform(mockContext);
					assertFalse("Resource (1) still exists in its initial location", resource1_file.exists());
					assertTrue("Resource (1) was not moved to its target location", resource1_expected_file.exists());
				}
			}
			
			public class WhenMimetypeNotFoundInPolicy {
				
				@Before
				public void setUp() throws Exception {
					sipResources.add(resource2);
					
					when(mockSIP.getResources()).thenReturn(sipResources);
					when(mockPolicyLoader.loadPersistencePolicies(any(StreamSource.class), same(mockSIP), eq(datasetName))).thenReturn(policies);
				}
				
				@Test
				public void resultShouldBeTrue() throws DepositException {
					boolean result = persist.perform(mockContext);
					assertTrue("Result should be true", result);
				}
				
				@Test
				public void resourceFileShouldBeSetToDefaultLocation() throws DepositException {
					File resource2_expected_file = new File(new File(resourcesTargetDir, policyTarget_default), resource2_filename);
					persist.perform(mockContext);
					assertEquals("Final resource (2) file different from expected", resource2_expected_file, resource2.getFile());
				}
				
				@Test
				public void resourceFileShouldBeMovedFromInitialLocationToTargetLocation( ) throws DepositException {
					File resource2_expected_file = new File(new File(resourcesTargetDir, policyTarget_default), resource2_filename);
					persist.perform(mockContext);
					assertFalse("Resource (2) still exists in its initial location", resource2_file.exists());
					assertTrue("Resource (2) was not moved to its target location", resource2_expected_file.exists());
				}
			}
			
			public class WhenPolicyCannotBeLoaded {
				
				@Rule
				public ExpectedException exceptionCheck = ExpectedException.none();
				
				@Before
				public void setUp() throws Exception {
					SaxonApiException exceptionToThrow = new SaxonApiException("some issue");
					
					sipResources.add(resource2);
					
					when(mockSIP.getResources()).thenReturn(sipResources);
					when(mockPolicyLoader.loadPersistencePolicies(any(StreamSource.class), same(mockSIP), eq(datasetName))).thenThrow(exceptionToThrow);
				}
				
				@Test
				public void shouldThrowDepositException() throws DepositException, SaxonApiException {
					exceptionCheck.expect(DepositException.class);
					persist.perform(mockContext);
				}
				
				@Test
				public void resourceFileShouldBeUnchanged() {
					try {
						persist.perform(mockContext);
					} catch (DepositException e) {
						// not what is being tested in this particular test - was already tested before
					}
					assertEquals("Resource (2) file location shouldn't have been changed", resource2_file, resource2.getFile());
				}
				
				@Test
				public void resourceFileShouldBeMovedFromInitialLocationToTargetLocation( ) {
					File resource2_target_file = new File(new File(resourcesTargetDir, policyTarget_default), resource2_filename);
					try {
						persist.perform(mockContext);
					} catch (DepositException e) {
						// not what is being tested in this particular test - was already tested before
					}
					assertTrue("Resource (2) shouldn't have been moved from its initial location", resource2_file.exists());
					assertFalse("Resource (2) shouldn't have been moved to a different location", resource2_target_file.exists());
				}
			}
			
			public class WhenPolicyFileIsInvalid {
				
				@Rule
				public ExpectedException exceptionCheck = ExpectedException.none();
				
				@Before
				public void setUp() throws Exception {
					IllegalStateException exceptionToThrow = new IllegalStateException("some issue with the policy file");
					
					sipResources.add(resource1);
					
					when(mockSIP.getResources()).thenReturn(sipResources);
					when(mockPolicyLoader.loadPersistencePolicies(any(StreamSource.class), same(mockSIP), eq(datasetName))).thenThrow(exceptionToThrow);
				}
				
				@Test
				public void shouldThrowDepositException() throws DepositException, SaxonApiException {
					exceptionCheck.expect(DepositException.class);
					persist.perform(mockContext);
				}
				
				@Test
				public void resourceFileShouldBeUnchanged() {
					try {
						persist.perform(mockContext);
					} catch (DepositException e) {
						// not what is being tested in this particular test - was already tested before
					}
					assertEquals("Resource (1) file location shouldn't have been changed", resource1_file, resource1.getFile());
				}
				
				@Test
				public void resourceFileShouldBeMovedFromInitialLocationToTargetLocation( ) {
					File resource1_target_file = new File(new File(resourcesTargetDir, policyTarget_pdf), resource1_filename);
					try {
						persist.perform(mockContext);
					} catch (DepositException e) {
						// not what is being tested in this particular test - was already tested before
					}
					assertTrue("Resource (1) shouldn't have been moved from its initial location", resource1_file.exists());
					assertFalse("Resource (1) shouldn't have been moved to a different location", resource1_target_file.exists());
				}
			}
			
			public class WhenTargetDirectoryCannotBeCreated {
				
				@Rule
				public ExpectedException exceptionCheck = ExpectedException.none();
				
				@Before
				public void setUp() throws Exception {
					sipResources.add(resource1);
					
					when(mockSIP.getResources()).thenReturn(sipResources);
					when(mockPolicyLoader.loadPersistencePolicies(any(StreamSource.class), same(mockSIP), eq(datasetName))).thenReturn(policies);
					
					resourcesTargetDir.setReadOnly();
				}
				
				@Test
				public void shouldThrowDepositException() throws DepositException {
					exceptionCheck.expect(DepositException.class);
					persist.perform(mockContext);
				}
				
				@Test
				public void resourceFileShouldBeUnchanged() {
					try {
						persist.perform(mockContext);
					} catch (DepositException e) {
						// not what is being tested in this particular test - was already tested before
					}
					assertEquals("Resource (1) file location shouldn't have been changed", resource1_file, resource1.getFile());
				}
				
				@Test
				public void resourceFileShouldBeMovedFromInitialLocationToTargetLocation( ) {
					File resource1_target_file = new File(new File(resourcesTargetDir, policyTarget_pdf), resource1_filename);
					try {
						persist.perform(mockContext);
					} catch (DepositException e) {
						// not what is being tested in this particular test - was already tested before
					}
					assertTrue("Resource (1) shouldn't have been moved from its initial location", resource1_file.exists());
					assertFalse("Resource (1) shouldn't have been moved to a different location", resource1_target_file.exists());
				}
			}
			
			public class WhenTargetFileAlreadyExists {

				@Rule
				public ExpectedException exceptionCheck = ExpectedException.none();
				
				@Before
				public void setUp() throws Exception {
					sipResources.add(resource1);
					
					when(mockSIP.getResources()).thenReturn(sipResources);
					when(mockPolicyLoader.loadPersistencePolicies(any(StreamSource.class), same(mockSIP), eq(datasetName))).thenReturn(policies);
					
					File resource1_target_folder = new File(resourcesTargetDir, policyTarget_pdf);
					File resource1_expected_file = new File(resource1_target_folder, resource1_filename);
					Files.createDirectories(resource1_target_folder.toPath());
					Files.createFile(resource1_expected_file.toPath());
				}
				
				@Test
				public void shouldThrowDepositException() throws DepositException {
					exceptionCheck.expect(DepositException.class);
					persist.perform(mockContext);
				}
				
				@Test
				public void resourceFileShouldBeUnchanged() {
					try {
						persist.perform(mockContext);
					} catch (DepositException e) {
						// not what is being tested in this particular test - was already tested before
					}
					assertEquals("Resource (1) file location shouldn't have been changed", resource1_file, resource1.getFile());
				}
				
				@Test
				public void resourceFileShouldNotBeMovedFromInitialLocation( ) {
					try {
						persist.perform(mockContext);
					} catch (DepositException e) {
						// not what is being tested in this particular test - was already tested before
					}
					assertTrue("Resource (1) shouldn't have been moved from its initial location", resource1_file.exists());
				}
			}
			
			public class WhenInitialFileCannotBeDeleted {

				@Rule
				public ExpectedException exceptionCheck = ExpectedException.none();
				
				@Before
				public void setUp() throws Exception {
					sipResources.add(resource1);
					
					when(mockSIP.getResources()).thenReturn(sipResources);
					when(mockPolicyLoader.loadPersistencePolicies(any(StreamSource.class), same(mockSIP), eq(datasetName))).thenReturn(policies);
					
					resourcesInitialDir.setReadOnly();
				}
				
				@Test
				public void shouldThrowDepositException() throws DepositException {
					exceptionCheck.expect(DepositException.class);
					persist.perform(mockContext);
				}
				
				@Test
				public void resourceFileShouldBeUnchanged() {
					try {
						persist.perform(mockContext);
					} catch (DepositException e) {
						// not what is being tested in this particular test - was already tested before
					}
					assertEquals("Resource (1) file location shouldn't have been changed", resource1_file, resource1.getFile());
				}
				
				@Test
				public void resourceFileShouldBeMovedFromInitialLocationToTargetLocation( ) {
					File resource1_target_file = new File(new File(resourcesTargetDir, policyTarget_pdf), resource1_filename);
					try {
						persist.perform(mockContext);
					} catch (DepositException e) {
						// not what is being tested in this particular test - was already tested before
					}
					assertTrue("Resource (1) shouldn't have been moved from its initial location", resource1_file.exists());
					assertFalse("Resource (1) shouldn't have been moved to a different location", resource1_target_file.exists());
				}
			}
			
			public class WhenTargetFileCannotBeWritten {
				
				@Rule
				public ExpectedException exceptionCheck = ExpectedException.none();
				
				@Before
				public void setUp() throws Exception {
					sipResources.add(resource1);
					
					when(mockSIP.getResources()).thenReturn(sipResources);
					when(mockPolicyLoader.loadPersistencePolicies(any(StreamSource.class), same(mockSIP), eq(datasetName))).thenReturn(policies);
					
					File resource1_target_folder = new File(resourcesTargetDir, policyTarget_pdf);
					Files.createDirectories(resource1_target_folder.toPath());
					resource1_target_folder.createNewFile();
					resource1_target_folder.setReadOnly();
				}
				
				@Test
				public void shouldThrowDepositException() throws DepositException {
					exceptionCheck.expect(DepositException.class);
					persist.perform(mockContext);
				}
				
				@Test
				public void resourceFileShouldBeUnchanged() {
					try {
						persist.perform(mockContext);
					} catch (DepositException e) {
						// not what is being tested in this particular test - was already tested before
					}
					assertEquals("Resource (1) file location shouldn't have been changed", resource1_file, resource1.getFile());
				}
				
				@Test
				public void resourceFileShouldBeMovedFromInitialLocationToTargetLocation( ) {
					File resource1_target_file = new File(new File(resourcesTargetDir, policyTarget_pdf), resource1_filename);
					try {
						persist.perform(mockContext);
					} catch (DepositException e) {
						// not what is being tested in this particular test - was already tested before
					}
					assertTrue("Resource (1) shouldn't have been moved from its initial location", resource1_file.exists());
					assertFalse("Resource (1) shouldn't have been moved to a different location", resource1_target_file.exists());
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
					when(mockPolicyLoader.loadPersistencePolicies(any(StreamSource.class), same(mockSIP), eq(datasetName))).thenReturn(policies);
				}
				
				@Test
				public void resultShouldBeTrue() throws DepositException {
					boolean result = persist.perform(mockContext);
					assertTrue("Result should be true", result);
				}
				
				@Test
				public void resourceFilesShouldBeSetToMatchedMimetypeLocations() throws DepositException {
					File resource1_expected_file = new File(new File(resourcesTargetDir, policyTarget_pdf), resource1_filename);
					File resource3_expected_file = new File(new File(resourcesTargetDir, policyTarget_text), resource3_filename);
					persist.perform(mockContext);
					assertEquals("Final resource (1) File different from expected", resource1_expected_file, resource1.getFile());
					assertEquals("Final resource (3) File different from expected", resource3_expected_file, resource3.getFile());
				}
				
				@Test
				public void resourceFilesShouldBeMovedFromInitialLocationsToTargetLocations( ) throws DepositException {
					File resource1_expected_file = new File(new File(resourcesTargetDir, policyTarget_pdf), resource1_filename);
					File resource3_expected_file = new File(new File(resourcesTargetDir, policyTarget_text), resource3_filename);
					persist.perform(mockContext);
					assertFalse("Resource (1) still exists in its initial location", resource1_file.exists());
					assertTrue("Resource (1) was not moved to its target location", resource1_expected_file.exists());
					assertFalse("Resource (3) still exists in its initial location", resource3_file.exists());
					assertTrue("Resource (3) was not moved to its target location", resource3_expected_file.exists());
				}
			}
			
			public class WhenNotAllMimetypesFoundInPolicy {
				
				@Before
				public void setUp() throws Exception {
					sipResources.add(resource1);
					sipResources.add(resource2);
					
					when(mockSIP.getResources()).thenReturn(sipResources);
					when(mockPolicyLoader.loadPersistencePolicies(any(StreamSource.class), same(mockSIP), eq(datasetName))).thenReturn(policies);
				}
				
				@Test
				public void resultShouldBeTrue() throws DepositException, SaxonApiException {
					boolean result = persist.perform(mockContext);
					assertTrue("Result should be true", result);
				}
				
				@Test
				public void resourceFilesShouldBeSetToMatchedMimetypeLocationsOrDefaultLocation() throws DepositException, SaxonApiException {
					File resource1_expected_file = new File(new File(resourcesTargetDir, policyTarget_pdf), resource1_filename);
					File resource2_expected_file = new File(new File(resourcesTargetDir, policyTarget_default), resource2_filename);
					persist.perform(mockContext);
					assertEquals("Final resource (1) File different from expected", resource1_expected_file, resource1.getFile());
					assertEquals("Final resource (2) file different from expected", resource2_expected_file, resource2.getFile());
				}
				
				@Test
				public void resourceFilesShouldBeMovedFromInitialLocationsToTargetLocations( ) throws DepositException {
					File resource1_expected_file = new File(new File(resourcesTargetDir, policyTarget_pdf), resource1_filename);
					File resource2_expected_file = new File(new File(resourcesTargetDir, policyTarget_default), resource2_filename);
					persist.perform(mockContext);
					assertFalse("Resource (1) still exists in its initial location", resource1_file.exists());
					assertTrue("Resource (1) was not moved to its target location", resource1_expected_file.exists());
					assertFalse("Resource (2) still exists in its initial location", resource2_file.exists());
					assertTrue("Resource (2) was not moved to its target location", resource2_expected_file.exists());
				}
			}
		}
	}
}
