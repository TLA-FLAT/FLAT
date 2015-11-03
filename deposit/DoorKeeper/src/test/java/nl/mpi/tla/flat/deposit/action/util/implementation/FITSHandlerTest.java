package nl.mpi.tla.flat.deposit.action.util.implementation;

import static org.junit.Assert.*;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import static org.mockito.Mockito.*;

import edu.harvard.hul.ois.fits.Fits;
import edu.harvard.hul.ois.fits.FitsOutput;
import edu.harvard.hul.ois.fits.exceptions.FitsException;
import edu.harvard.hul.ois.fits.identity.FitsIdentity;

public class FITSHandlerTest {

	private FITSHandler fitsHandler;
	
	@Mock Fits mockFits;
	@Mock FitsOutput mockFitsOutput;
	@Mock FitsIdentity mockFitsIdentity;

	@Mock FileTypeChecker mockFileTypeChecker;
	
	@Mock File mockFileToCheck;
	
	@Before
	public void setUp() throws Exception {
		
		MockitoAnnotations.initMocks(this);
		
		fitsHandler = new FITSHandler(mockFits, mockFileTypeChecker);
	}

	@Test
	public void fileHaveIdentity_Acceptable() throws FitsException {
		
		final String goodMimetype = "text/plain";
		
		List<FitsIdentity> fitsIdentities = new ArrayList<>();
		fitsIdentities.add(mockFitsIdentity);
		
		when(mockFits.examine(mockFileToCheck)).thenReturn(mockFitsOutput);
		when(mockFitsOutput.getIdentities()).thenReturn(fitsIdentities);
		when(mockFitsIdentity.getMimetype()).thenReturn(goodMimetype);
		when(mockFileTypeChecker.isMimetypeInAcceptableList(goodMimetype)).thenReturn(Boolean.TRUE);
		
		boolean result = fitsHandler.isFileAcceptable(mockFileToCheck);
		
		assertTrue("Result should be true", result);
	}
	
	@Test
	public void fileHaveIdentity_Unacceptable() throws FitsException {
		
		final String badMimetype = "text/flat";
		
		List<FitsIdentity> fitsIdentities = new ArrayList<>();
		fitsIdentities.add(mockFitsIdentity);
		
		when(mockFits.examine(mockFileToCheck)).thenReturn(mockFitsOutput);
		when(mockFitsOutput.getIdentities()).thenReturn(fitsIdentities);
		when(mockFitsIdentity.getMimetype()).thenReturn(badMimetype);
		when(mockFileTypeChecker.isMimetypeInAcceptableList(badMimetype)).thenReturn(Boolean.FALSE);
		
		boolean result = fitsHandler.isFileAcceptable(mockFileToCheck);
		
		assertFalse("Result should be false", result);
	}

	@Test
	public void fileHasNoIdentities() {
		fail("not tested yet");
	}
	
	@Test
	public void fileHasMultipleIdentities() {
		fail("not tested yet");
	}
}
