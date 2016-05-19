package nl.mpi.tla.flat.deposit.action.fits.util;

import static org.junit.Assert.*;
import static org.powermock.api.support.membermodification.MemberMatcher.method;
import static org.powermock.api.support.membermodification.MemberModifier.stub;
import static org.mockito.Mockito.*;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import javax.xml.transform.stream.StreamSource;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import net.sf.saxon.s9api.SaxonApiException;
import nl.mpi.tla.flat.deposit.action.fits.util.FileTypeChecker;
import nl.mpi.tla.flat.deposit.action.fits.util.MimetypesLoader;

@RunWith(PowerMockRunner.class)
@PrepareForTest({MimetypesLoader.class})
public class FileTypeCheckerTest {

	@Mock MimetypesLoader mockMimetypesLoader;
	
	private List<String> acceptedMimetypes;
	
	private FileTypeChecker fileTypeChecker;
	
	@Before
	public void setUp() throws Exception {
		
		acceptedMimetypes = new ArrayList<>();
		acceptedMimetypes.add("application/pdf");
		
		File mimetypesFile = new File(getClass().getClassLoader().getResource("policies/fits-mimetypes.xml").getFile());
		fileTypeChecker = FileTypeChecker.getNewFileTypeChecker(mimetypesFile);
		
		stub(method(MimetypesLoader.class, "getNewMimetypesLoader")).toReturn(mockMimetypesLoader);
	}

	@Test
	public void mimetype_AcceptableOrNot() throws SaxonApiException {
		
		String goodMimetype = "application/pdf";
		String badMimetype = "bla/bla";
		
		when(mockMimetypesLoader.loadMimetypes(any(StreamSource.class))).thenReturn(acceptedMimetypes);
		
		boolean result = fileTypeChecker.isMimetypeInAcceptableList(goodMimetype);
		assertTrue("Result (1) should be true", result);
		
		result = fileTypeChecker.isMimetypeInAcceptableList(badMimetype);
		assertFalse("Result (2) should be false", result);
	}
}
