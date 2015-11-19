package nl.mpi.tla.flat.deposit.action.fits.util;

import static org.junit.Assert.*;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import javax.xml.transform.Source;
import javax.xml.transform.stream.StreamSource;

import org.junit.Before;
import org.junit.Test;

import net.sf.saxon.s9api.SaxonApiException;
import nl.mpi.tla.flat.deposit.action.fits.util.MimetypesLoader;

public class MimetypesLoaderTest {

	private MimetypesLoader mimetypesLoader;
	
	@Before
	public void setUp() throws Exception {
		
		mimetypesLoader = MimetypesLoader.getNewMimetypesLoader();
	}

	@Test
	public void test() throws SaxonApiException {
		
		List<String> expectedMimetypes = new ArrayList<>();
		expectedMimetypes.add("application/pdf");
		expectedMimetypes.add("text/plain");
		expectedMimetypes.add("image/jpg");
		
		File mimetypesFile = new File(getClass().getClassLoader().getResource("policies/fits-mimetypes.xml").getFile());
		
		Source mimetypesSource = new StreamSource(mimetypesFile);
		
		List<String> retrievedMimetypes = mimetypesLoader.loadMimetypes(mimetypesSource);
		
		assertNotNull("Retrieved list should not be null", retrievedMimetypes);
		assertFalse("Retrieved list should not be empty", retrievedMimetypes.isEmpty());
		assertTrue("Mimetypes list different from expected", retrievedMimetypes.containsAll(expectedMimetypes));
	}
}
