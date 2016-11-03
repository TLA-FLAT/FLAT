package nl.mpi.tla.flat.deposit.action;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;
import static org.powermock.api.support.membermodification.MemberMatcher.method;
import static org.powermock.api.support.membermodification.MemberModifier.stub;

import java.io.File;
import java.io.IOException;
import java.net.URI;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import net.handle.hdllib.HandleException;
import net.sf.saxon.s9api.XdmAtomicValue;
import net.sf.saxon.s9api.XdmValue;
import nl.mpi.handle.util.HandleManager;
import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.sip.Resource;
import nl.mpi.tla.flat.deposit.sip.SIPInterface;
import nl.mpi.tla.flat.deposit.action.handle.util.HandleManagerFactory;

@RunWith(PowerMockRunner.class)
@PrepareForTest({HandleManagerFactory.class})
public class TLAHandleCreationTest {

	private TLAHandleCreation tlaHandleCreation;
	
	@Rule
	public ExpectedException exceptionCheck = ExpectedException.none();
	
	@Mock Context mockContext;
	@Mock SIPInterface mockSIP;
	@Mock Resource mockResource1;
	@Mock Resource mockResource2;
	@Mock Resource mockResource3;
	@Mock File mockFile_Sip;
	@Mock File mockFile_Resource1;
	@Mock File mockFile_Resource2;
	
	@Mock HandleManager mockHandleManager;
	
	private final String fedoraServer = "https://some.server/fedora";
	private String handlePrefix = "12345";
	private String handleAdminKeyFilePath = "/lat/handle/key";
	private String handleAdminUserHandleIndex = "000";
	private String handleAdminUserHandle = "AA/12345";
	private String handleAdminPassword = "pass";

	
	
	// SIP
	private final String sip_PidStr = "hdl:123/ABCDE";
	private final URI sip_Pid = URI.create(sip_PidStr);
	private final String sip_FidStr = "lat:123_ABCDE";
	private final String sip_DsidStr = "CMD";
	private final String sip_Asof = "201512101759";
	private final String sip_FidCompleteStr = sip_FidStr + "#" + sip_DsidStr + "@" + sip_Asof;
	private final URI sip_FidComplete = URI.create(sip_FidCompleteStr);
	private final String sip_HandleTargetStr = fedoraServer + "/objects/" + sip_FidStr + "/datastreams/" + sip_DsidStr + "/content?asOfDateTime=" + sip_Asof;
	private final URI sip_HandleTarget = URI.create(sip_HandleTargetStr);
	
	// Resource 1
	private final String resource1_PidStr = "hdl:234/BCDEF";
	private final URI resource1_Pid = URI.create(resource1_PidStr);
	private final String resource1_FidStr = "lat:234_BCDEF";
	private final String resource1_DsidStr = "OBJ";
	private final String resource1_Asof = "201512101759";
	private final String resource1_FidCompleteStr = resource1_FidStr + "#" + resource1_DsidStr + "@" + resource1_Asof;
	private final URI resource1_FidComplete = URI.create(resource1_FidCompleteStr);
	private final String resource1_HandleTargetStr = fedoraServer + "/objects/" + resource1_FidStr + "/datastreams/" + resource1_DsidStr + "/content?asOfDateTime=" + resource1_Asof;
	private final URI resource1_HandleTarget = URI.create(resource1_HandleTargetStr);
	
	// Resource 2
	private final String resource2_PidStr = "hdl:345/CDEFG";
	private final URI resource2_Pid = URI.create(resource2_PidStr);
	private final String resource2_FidStr = "lat:345_CDEFG";
	private final String resource2_DsidStr = "OBJ";
	private final String resource2_Asof = "201512101759";
	private final String resource2_FidCompleteStr = resource2_FidStr + "#" + resource2_DsidStr + "@" + resource2_Asof;
	private final URI resource2_FidComplete = URI.create(resource2_FidCompleteStr);
	private final String resource2_HandleTargetStr = fedoraServer + "/objects/" + resource2_FidStr + "/datastreams/" + resource2_DsidStr + "/content?asOfDateTime=" + resource2_Asof;
	private final URI resource2_HandleTarget = URI.create(resource2_HandleTargetStr);
	
	
	@Before
	public void setUp() throws Exception {
		
		Map<String, XdmValue> parameters = new HashMap<>();
		parameters.put("fedoraServer", new XdmAtomicValue(fedoraServer));
		parameters.put("handlePrefix", new XdmAtomicValue(handlePrefix));
		parameters.put("handleAdminKeyFilePath", new XdmAtomicValue(handleAdminKeyFilePath));
		parameters.put("handleAdminUserHandleIndex", new XdmAtomicValue(handleAdminUserHandleIndex));
		parameters.put("handleAdminUserHandle", new XdmAtomicValue(handleAdminUserHandle));
		parameters.put("handleAdminPassword", new XdmAtomicValue(handleAdminPassword));
		
		MockitoAnnotations.initMocks(this);
		
		tlaHandleCreation = new TLAHandleCreation();
		tlaHandleCreation.setParameters(parameters);
	}

	@Test
	public void createHandle_sipHasNoPID() throws DepositException {

		Set<Resource> sipResources = new HashSet<>();
		sipResources.add(mockResource1);
		sipResources.add(mockResource2);
		
		exceptionCheck.expect(DepositException.class);
		DepositException exceptionToThrow = new DepositException();
		
		when(mockContext.getSIP()).thenReturn(mockSIP);
		when(mockSIP.getFID()).thenReturn(sip_FidComplete);
		when(mockSIP.getPID()).thenThrow(exceptionToThrow);

		tlaHandleCreation.perform(mockContext);
	}

	@Test
	public void createHandle_resourceHasNoPID() throws DepositException, HandleException, IOException {
		
		Set<Resource> sipResources = new HashSet<>();
		sipResources.add(mockResource1);
		sipResources.add(mockResource2);

		DepositException exceptionToThrow = new DepositException();

		stub(method(HandleManagerFactory.class, "getNewHandleManager", String.class, String.class, String.class, String.class, String.class)).toReturn(mockHandleManager);
		
		when(mockContext.getSIP()).thenReturn(mockSIP);
		when(mockSIP.getFID()).thenReturn(sip_FidComplete);
		when(mockSIP.getPID()).thenReturn(sip_Pid);

		when(mockSIP.getResources()).thenReturn(sipResources);
		when(mockSIP.getBase()).thenReturn(mockFile_Sip);
		
		when(mockResource1.getFID()).thenReturn(resource1_FidComplete);
		when(mockResource1.getPID()).thenThrow(exceptionToThrow);
		when(mockResource1.getFile()).thenReturn(mockFile_Resource1);
		
		when(mockResource2.getFID()).thenReturn(resource2_FidComplete);
		when(mockResource2.getPID()).thenReturn(resource2_Pid);
		when(mockResource2.getFile()).thenReturn(mockFile_Resource2);

		boolean result = tlaHandleCreation.perform(mockContext);
		
		verify(mockHandleManager).assignHandle(mockFile_Sip, sip_Pid, sip_HandleTarget);
		verify(mockHandleManager).assignHandle(mockFile_Resource2, resource2_Pid, resource2_HandleTarget);
		
		assertFalse("Result should be false", result);
	}
	
	@Test
	public void createHandle_sipHasNoResources() throws DepositException, HandleException, IOException {
		
		Set<Resource> noResources = new HashSet<>();

		stub(method(HandleManagerFactory.class, "getNewHandleManager", String.class, String.class, String.class, String.class, String.class)).toReturn(mockHandleManager);
		
		when(mockContext.getSIP()).thenReturn(mockSIP);
		when(mockSIP.getFID()).thenReturn(sip_FidComplete);
		when(mockSIP.getPID()).thenReturn(sip_Pid);

		when(mockSIP.getResources()).thenReturn(noResources);
		when(mockSIP.getBase()).thenReturn(mockFile_Sip);

		boolean result = tlaHandleCreation.perform(mockContext);
		
		verify(mockHandleManager).assignHandle(mockFile_Sip, sip_Pid, sip_HandleTarget);
		
		assertTrue("Result should be true", result);
	}
	
	@Test
	public void createHandle_everythingHasPID() throws DepositException, HandleException, IOException {
		
		Set<Resource> sipResources = new HashSet<>();
		sipResources.add(mockResource1);
		sipResources.add(mockResource2);
		

		stub(method(HandleManagerFactory.class, "getNewHandleManager", String.class, String.class, String.class, String.class, String.class)).toReturn(mockHandleManager);
		
		when(mockContext.getSIP()).thenReturn(mockSIP);
		when(mockSIP.getFID()).thenReturn(sip_FidComplete);
		when(mockSIP.getPID()).thenReturn(sip_Pid);

		when(mockSIP.getResources()).thenReturn(sipResources);
		when(mockSIP.getBase()).thenReturn(mockFile_Sip);
		
		when(mockResource1.getFID()).thenReturn(resource1_FidComplete);
		when(mockResource1.getPID()).thenReturn(resource1_Pid);
		when(mockResource1.getFile()).thenReturn(mockFile_Resource1);
		
		when(mockResource2.getFID()).thenReturn(resource2_FidComplete);
		when(mockResource2.getPID()).thenReturn(resource2_Pid);
		when(mockResource2.getFile()).thenReturn(mockFile_Resource2);

		boolean result = tlaHandleCreation.perform(mockContext);
		
		verify(mockHandleManager).assignHandle(mockFile_Sip, sip_Pid, sip_HandleTarget);
		verify(mockHandleManager).assignHandle(mockFile_Resource1, resource1_Pid, resource1_HandleTarget);
		verify(mockHandleManager).assignHandle(mockFile_Resource2, resource2_Pid, resource2_HandleTarget);
		
		assertTrue("Result should be true", result);
	}
	
	@Test
	public void createHandle_ExceptionGettingHandleManager() throws DepositException {
		
		Set<Resource> sipResources = new HashSet<>();
		sipResources.add(mockResource1);
		sipResources.add(mockResource2);
		
		exceptionCheck.expect(DepositException.class);
		IOException exceptionToThrow = new IOException();

		stub(method(HandleManagerFactory.class, "getNewHandleManager", String.class, String.class, String.class, String.class, String.class)).toThrow(exceptionToThrow);
		
		tlaHandleCreation.perform(mockContext);
	}
	
	@Test
	public void createHandle_ExceptionAssigningHandleToSip() throws DepositException, HandleException, IOException {
		
		Set<Resource> sipResources = new HashSet<>();
		sipResources.add(mockResource1);
		sipResources.add(mockResource2);
		
		exceptionCheck.expect(DepositException.class);
		HandleException exceptionToThrow = new HandleException(HandleException.CANNOT_CONNECT_TO_SERVER);

		stub(method(HandleManagerFactory.class, "getNewHandleManager", String.class, String.class, String.class, String.class, String.class)).toReturn(mockHandleManager);
		
		when(mockContext.getSIP()).thenReturn(mockSIP);
		when(mockSIP.getFID()).thenReturn(sip_FidComplete);
		when(mockSIP.getPID()).thenReturn(sip_Pid);

		when(mockSIP.getResources()).thenReturn(sipResources);
		when(mockSIP.getBase()).thenReturn(mockFile_Sip);

		when(mockHandleManager.assignHandle(mockFile_Sip, sip_Pid, sip_HandleTarget)).thenThrow(exceptionToThrow);
		
		tlaHandleCreation.perform(mockContext);
	}
	
	@Test
	public void createHandle_ExceptionAssigningHandleToResource() throws DepositException, HandleException, IOException {
		
		Set<Resource> sipResources = new HashSet<>();
		sipResources.add(mockResource1);
		sipResources.add(mockResource2);
		
		HandleException exceptionToThrow = new HandleException(HandleException.CANNOT_CONNECT_TO_SERVER);
		

		stub(method(HandleManagerFactory.class, "getNewHandleManager", String.class, String.class, String.class, String.class, String.class)).toReturn(mockHandleManager);
		
		when(mockContext.getSIP()).thenReturn(mockSIP);
		when(mockSIP.getFID()).thenReturn(sip_FidComplete);
		when(mockSIP.getPID()).thenReturn(sip_Pid);

		when(mockSIP.getResources()).thenReturn(sipResources);
		when(mockSIP.getBase()).thenReturn(mockFile_Sip);
		
		when(mockResource1.getFID()).thenReturn(resource1_FidComplete);
		when(mockResource1.getPID()).thenReturn(resource1_Pid);
		when(mockResource1.getFile()).thenReturn(mockFile_Resource1);
		
		when(mockResource2.getFID()).thenReturn(resource2_FidComplete);
		when(mockResource2.getPID()).thenReturn(resource2_Pid);
		when(mockResource2.getFile()).thenReturn(mockFile_Resource2);
		
		when(mockHandleManager.assignHandle(mockFile_Resource1, resource1_Pid, resource1_HandleTarget)).thenThrow(exceptionToThrow);

		boolean result = tlaHandleCreation.perform(mockContext);
		
		verify(mockHandleManager).assignHandle(mockFile_Sip, sip_Pid, sip_HandleTarget);
		verify(mockHandleManager).assignHandle(mockFile_Resource2, resource2_Pid, resource2_HandleTarget);
		
		assertFalse("Result should be false", result);
	}
}
