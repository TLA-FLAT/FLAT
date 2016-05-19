package nl.mpi.tla.flat.deposit.action.persist.util;

import static org.junit.Assert.*;

import java.io.File;

import org.junit.Before;
import org.junit.Test;
import org.w3c.dom.Document;

import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.util.Saxon;

public class PersistDatasetNameRetrieverTest {

	private Document testSipRecord;
	private String xpathDatasetName = "replace(//*[name()='MdSelfLink'], 'hdl:11142/','')";
	
	private PersistDatasetNameRetriever datasetNameRetriever;
	
	
	@Before
	public void setUp() throws Exception {
		datasetNameRetriever = new PersistDatasetNameRetriever();
		testSipRecord = Saxon.buildDOM(new File(getClass().getClassLoader().getResource("test_sip/Progressive_Corpus.cmdi").getFile()));
	}

	@Test
	public void test() throws DepositException {
		String expectedDatasetName = "DD52217D-395E-4ECC-BC97-27439D325215";
		String retrievedDatasetName = datasetNameRetriever.getDatasetName(testSipRecord, xpathDatasetName);
		assertEquals("Retrieved dataset name different from expected", expectedDatasetName, retrievedDatasetName);
	}

}
