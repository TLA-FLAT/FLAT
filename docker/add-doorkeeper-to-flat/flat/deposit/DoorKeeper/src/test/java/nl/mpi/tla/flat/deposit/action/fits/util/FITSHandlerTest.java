package nl.mpi.tla.flat.deposit.action.fits.util;

import static org.junit.Assert.*;

import java.io.File;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import net.sf.saxon.s9api.XdmNode;
import net.sf.saxon.s9api.XdmValue;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import static org.powermock.api.support.membermodification.MemberMatcher.method;
import static org.powermock.api.support.membermodification.MemberModifier.stub;

import static org.mockito.Mockito.*;

import nl.mpi.tla.flat.deposit.action.fits.util.FITSHandler;
import nl.mpi.tla.flat.deposit.action.fits.util.FileTypeChecker;

@RunWith(PowerMockRunner.class)
@PrepareForTest({FileTypeChecker.class})
public class FITSHandlerTest {

	private FITSHandler fitsHandler;
	
	@Rule
	public ExpectedException exceptionCheck = ExpectedException.none();
	
	@Mock URL mockFitsService;
	@Mock XdmNode mockFitsOutput;
	@Mock XdmValue mockFitsIdentity;
	@Mock XdmValue mockOtherFitsIdentity;
	
	@Mock FileTypeChecker mockFileTypeChecker;
	
	@Mock File mockFileToCheck;
	
	@Before
	public void setUp() throws Exception {
		
		MockitoAnnotations.initMocks(this);
		
		stub(method(FileTypeChecker.class, "getNewFileTypeChecker")).toReturn(mockFileTypeChecker);
		
		URL fitsService = new URL("http://localhost/fits/");
		String mimetypesFileLocation = "policies/fits-mimetypes.xml";
		
		fitsHandler = new FITSHandler(fitsService, mimetypesFileLocation);
	}
	
/*	
	@Test
	public void performFitsCheck_Exception() throws FitsException {
	
		exceptionCheck.expect(FitsException.class);
		FitsException expectedException = new FitsException();
		
		when(mockFits.examine(mockFileToCheck)).thenThrow(expectedException);
		
		fitsHandler.performFitsCheck(mockFileToCheck);
	}
	
	@Test
	public void getResultMimetype_Successful() {
		
		final String mimetype = "text/plain";
		
		List<FitsIdentity> fitsIdentities = new ArrayList<>();
		fitsIdentities.add(mockFitsIdentity);
		
		when(mockFitsOutput.getIdentities()).thenReturn(fitsIdentities);
		when(mockFitsIdentity.getMimetype()).thenReturn(mimetype);
		
		String retrievedMimetype = fitsHandler.getResultMimetype(mockFileToCheck, mockFitsOutput);
		
		assertEquals("Result different from expected", mimetype, retrievedMimetype);
	}
	
	@Test
	public void getResultMimetype_FitsOutputNull() {
		
		String retrievedMimetype = fitsHandler.getResultMimetype(mockFileToCheck, null);
		
		assertEquals("Result different from expected", "", retrievedMimetype);
	}
	
	@Test
	public void getResultMimetype_EmptyIdentities() {
		
		List<FitsIdentity> emptyFitsIdentities = new ArrayList<>();
		
		when(mockFitsOutput.getIdentities()).thenReturn(emptyFitsIdentities);
		
		String retrievedMimetype = fitsHandler.getResultMimetype(mockFileToCheck, mockFitsOutput);
		
		assertEquals("Result different from expected", "", retrievedMimetype);
	}
	
	@Test
	public void getResultMimetype_MultipleIdentities() {
		
		List<FitsIdentity> fitsIdentities = new ArrayList<>();
		fitsIdentities.add(mockFitsIdentity);
		fitsIdentities.add(mockOtherFitsIdentity);
		
		when(mockFitsOutput.getIdentities()).thenReturn(fitsIdentities);
		
		String retrievedMimetype = fitsHandler.getResultMimetype(mockFileToCheck, mockFitsOutput);
		
		assertEquals("Result different from expected", "", retrievedMimetype);
	}
*/	
	@Test
	public void mimetypeAcceptable() {
		
		final String goodMimetype = "text/plain";
		
		when(mockFileTypeChecker.isMimetypeInAcceptableList(goodMimetype)).thenReturn(Boolean.TRUE);
		
		boolean result = fitsHandler.isMimetypeAcceptable(goodMimetype);
		
		assertTrue("Result should be true", result);
	}
	@Test
	public void mimetypeNotAcceptable() {
		
		final String goodMimetype = "text/mountain";
		
		when(mockFileTypeChecker.isMimetypeInAcceptableList(goodMimetype)).thenReturn(Boolean.FALSE);
		
		boolean result = fitsHandler.isMimetypeAcceptable(goodMimetype);
		
		assertFalse("Result should be false", result);
	}
}
