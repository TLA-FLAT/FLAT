package nl.mpi.tla.flat.deposit.action;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.when;
import static org.powermock.api.support.membermodification.MemberMatcher.method;
import static org.powermock.api.support.membermodification.MemberModifier.stub;

import java.io.File;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import net.sf.saxon.s9api.SaxonApiException;
import net.sf.saxon.s9api.XdmAtomicValue;
import net.sf.saxon.s9api.XdmValue;
import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.Resource;
import nl.mpi.tla.flat.deposit.SIP;
import nl.mpi.tla.flat.deposit.action.util.FITSHandler;

@RunWith(PowerMockRunner.class)
@PrepareForTest({FITSHandler.class})
public class FITSTest {

	private FITS fits;
	
	@Mock Context mockContext;
	@Mock SIP mockSIP;
	@Mock Resource mockResource1;
	@Mock Resource mockResource2;
	@Mock File mockFile1;
	@Mock File mockFile2;
	@Mock FITSHandler mockFITSHandler;
	
	@Before
	public void setUp() throws SaxonApiException {
		
		Map<String, XdmValue> parameters = new HashMap<>();
		parameters.put("fits_home", new XdmAtomicValue("lib/fits-0.8.10"));
		
		MockitoAnnotations.initMocks(this);
		
		fits = new FITS();
		fits.setParameters(parameters);
	}
	
	@Test
	public void acceptFileType() throws DepositException {
		
		Set<Resource> sipResources = new HashSet<>();
		sipResources.add(mockResource1);
		
		when(mockContext.getSIP()).thenReturn(mockSIP);
		when(mockSIP.getResources()).thenReturn(sipResources);
		when(mockResource1.hasFile()).thenReturn(Boolean.TRUE);
		when(mockResource1.getFile()).thenReturn(mockFile1);
		when(mockFITSHandler.isFileAcceptable(mockFile1)).thenReturn(Boolean.TRUE);
		
		stub(method(FITSHandler.class, "getNewFITSHandler", String.class)).toReturn(mockFITSHandler);
		
		boolean result = fits.perform(mockContext);
		
		assertTrue("Result should be true", result);
	}

	@Test
	public void rejectFileType() throws DepositException {
		
		Set<Resource> sipResources = new HashSet<>();
		sipResources.add(mockResource1);
		
		when(mockContext.getSIP()).thenReturn(mockSIP);
		when(mockSIP.getResources()).thenReturn(sipResources);
		when(mockResource1.hasFile()).thenReturn(Boolean.TRUE);
		when(mockResource1.getFile()).thenReturn(mockFile1);
		when(mockFITSHandler.isFileAcceptable(mockFile1)).thenReturn(Boolean.FALSE);
		
		stub(method(FITSHandler.class, "getNewFITSHandler", String.class)).toReturn(mockFITSHandler);
		
		boolean result = fits.perform(mockContext);
		
		assertFalse("Result should be false", result);
	}
	
	//TODO Add some tests with multiple files to check
}
