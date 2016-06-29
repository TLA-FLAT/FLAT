package nl.mpi.tla.flat.deposit.action;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.when;
import static org.mockito.Mockito.verify;
import static org.powermock.api.support.membermodification.MemberMatcher.method;
import static org.powermock.api.support.membermodification.MemberModifier.stub;

import java.io.File;
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

import edu.harvard.hul.ois.fits.FitsOutput;
import edu.harvard.hul.ois.fits.exceptions.FitsConfigurationException;
import edu.harvard.hul.ois.fits.exceptions.FitsException;
import net.sf.saxon.s9api.SaxonApiException;
import net.sf.saxon.s9api.XdmAtomicValue;
import net.sf.saxon.s9api.XdmValue;
import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.Resource;
import nl.mpi.tla.flat.deposit.SIP;
import nl.mpi.tla.flat.deposit.action.fits.util.FITSHandler;

@RunWith(PowerMockRunner.class)
@PrepareForTest({FITSHandler.class})
public class FITSTest {

	private FITS fits;
	
	@Rule
	public ExpectedException exceptionCheck = ExpectedException.none();
	
	@Mock Context mockContext;
	@Mock SIP mockSIP;
	@Mock Resource mockResource1;
	@Mock Resource mockResource2;
	@Mock Resource mockResource3;
	@Mock File mockFile1;
	@Mock File mockFile3;
	@Mock FITSHandler mockFITSHandler;
	@Mock FitsOutput mockFitsOutput1;
	@Mock FitsOutput mockFitsOutput3;
	
	@Before
	public void setUp() throws SaxonApiException {
		
		Map<String, XdmValue> parameters = new HashMap<>();
		parameters.put("fits_home", new XdmAtomicValue("lib/fits-0.8.10"));
		
		MockitoAnnotations.initMocks(this);
		
		fits = new FITS();
		fits.setParameters(parameters);
	}
	
	@Test
	public void acceptFileType() throws DepositException, FitsException {
		
		final String goodMimetype = "text/plain";
		
		Set<Resource> sipResources = new HashSet<>();
		sipResources.add(mockResource1);
		
		when(mockContext.getSIP()).thenReturn(mockSIP);
		when(mockSIP.getResources()).thenReturn(sipResources);
		when(mockResource1.hasFile()).thenReturn(Boolean.TRUE);
		when(mockResource1.getFile()).thenReturn(mockFile1);
		when(mockFITSHandler.performFitsCheck(mockFile1)).thenReturn(mockFitsOutput1);
		when(mockFITSHandler.getResultMimetype(mockFile1, mockFitsOutput1)).thenReturn(goodMimetype);
		when(mockFITSHandler.isMimetypeAcceptable(goodMimetype)).thenReturn(Boolean.TRUE);
		
		stub(method(FITSHandler.class, "getNewFITSHandler", String.class, String.class)).toReturn(mockFITSHandler);
		
		boolean result = fits.perform(mockContext);
		
		verify(mockResource1).hasMime();
		verify(mockResource1).setMime(goodMimetype);
		
		assertTrue("Result should be true", result);
	}

	@Test
	public void rejectFileType() throws DepositException, FitsException {
		
		final String badMimetype = "text/plateau";
		
		Set<Resource> sipResources = new HashSet<>();
		sipResources.add(mockResource1);
		
		when(mockContext.getSIP()).thenReturn(mockSIP);
		when(mockSIP.getResources()).thenReturn(sipResources);
		when(mockResource1.hasFile()).thenReturn(Boolean.TRUE);
		when(mockResource1.getFile()).thenReturn(mockFile1);
		when(mockFITSHandler.performFitsCheck(mockFile1)).thenReturn(mockFitsOutput1);
		when(mockFITSHandler.getResultMimetype(mockFile1, mockFitsOutput1)).thenReturn(badMimetype);
		when(mockFITSHandler.isMimetypeAcceptable(badMimetype)).thenReturn(Boolean.FALSE);
		
		stub(method(FITSHandler.class, "getNewFITSHandler", String.class, String.class)).toReturn(mockFITSHandler);
		
		boolean result = fits.perform(mockContext);
		
		assertFalse("Result should be false", result);
	}
	
	@Test
	public void errorCreatingFits() throws DepositException {
		
		exceptionCheck.expect(DepositException.class);
		FitsConfigurationException exceptionToThrow = new FitsConfigurationException();
		
		stub(method(FITSHandler.class, "getNewFITSHandler", String.class, String.class)).toThrow(exceptionToThrow);
		
		fits.perform(mockContext);
	}
	
	@Test
	public void throwsFitsException() throws DepositException, FitsException {
	
		final FitsException expectedException = new FitsException();
		
		Set<Resource> sipResources = new HashSet<>();
		sipResources.add(mockResource1);
		
		when(mockContext.getSIP()).thenReturn(mockSIP);
		when(mockSIP.getResources()).thenReturn(sipResources);
		when(mockResource1.hasFile()).thenReturn(Boolean.TRUE);
		when(mockResource1.getFile()).thenReturn(mockFile1);
		when(mockFITSHandler.performFitsCheck(mockFile1)).thenThrow(expectedException);
		
		stub(method(FITSHandler.class, "getNewFITSHandler", String.class, String.class)).toReturn(mockFITSHandler);
		
		boolean result = fits.perform(mockContext);
		
		assertFalse("Result should be false", result);
	}
	
	@Test
	public void multipleFiles() throws DepositException, FitsException {
		
		final String goodMimetype = "text/plain";
		final String badMimetype = "text/lake";
		
		Set<Resource> sipResources = new HashSet<>();
		sipResources.add(mockResource1);
		sipResources.add(mockResource2);
		sipResources.add(mockResource3);
		
		when(mockContext.getSIP()).thenReturn(mockSIP);
		when(mockSIP.getResources()).thenReturn(sipResources);
		
		when(mockResource1.hasFile()).thenReturn(Boolean.TRUE);
		when(mockResource1.getFile()).thenReturn(mockFile1);
		when(mockFITSHandler.performFitsCheck(mockFile1)).thenReturn(mockFitsOutput1);
		when(mockFITSHandler.getResultMimetype(mockFile1, mockFitsOutput1)).thenReturn(goodMimetype);
		when(mockFITSHandler.isMimetypeAcceptable(goodMimetype)).thenReturn(Boolean.TRUE);
		
		when(mockResource2.hasFile()).thenReturn(Boolean.FALSE);
		
		when(mockResource3.hasFile()).thenReturn(Boolean.TRUE);
		when(mockResource3.getFile()).thenReturn(mockFile3);
		when(mockFITSHandler.performFitsCheck(mockFile3)).thenReturn(mockFitsOutput3);
		when(mockFITSHandler.getResultMimetype(mockFile3, mockFitsOutput3)).thenReturn(badMimetype);
		when(mockFITSHandler.isMimetypeAcceptable(badMimetype)).thenReturn(Boolean.FALSE);
		
		stub(method(FITSHandler.class, "getNewFITSHandler", String.class, String.class)).toReturn(mockFITSHandler);
		
		boolean result = fits.perform(mockContext);
		
		verify(mockResource1).hasMime();
		verify(mockResource1).setMime(goodMimetype);
		
		assertFalse("Result should be false", result);
	}
}
