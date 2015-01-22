/*
 * Copyright (C) 2014 The Language Archive - Max Planck Institute for Psycholinguistics
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package nl.mpi.tla.lat2fox;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Collection;
import java.util.ArrayList;
import java.util.List;
import java.util.TreeSet;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;
import joptsimple.OptionParser;
import joptsimple.OptionSet;
import net.sf.saxon.s9api.DocumentBuilder;
import net.sf.saxon.s9api.QName;
import net.sf.saxon.s9api.XdmAtomicValue;
import net.sf.saxon.s9api.XdmDestination;
import net.sf.saxon.s9api.XdmNode;
import net.sf.saxon.s9api.XsltExecutable;
import net.sf.saxon.s9api.XsltTransformer;
import nl.mpi.tla.schemanon.Message;
import nl.mpi.tla.schemanon.SchemAnon;
import nl.mpi.tla.schemanon.SaxonUtils;
import org.apache.commons.io.FileUtils;

/**
 * @author Menzo Windhouwer
 */
public class Main {
    
    private static void showHelp() {
        System.err.println("INF: lat2fox <options> -- <DIR>?");
        System.err.println("INF: <DIR>     source directory to recurse for CMD files (default: .)");
        System.err.println("INF: lat2fox options:");
        System.err.println("INF: -r=<FILE> load/store the relations map from/in this <FILE> (optional)");
        System.err.println("INF: -f=<DIR>  directory to store the FOX files (optional)");
        System.err.println("INF: -i=<DIR>  replace source <DIR> by this <DIR> in the FOX files (optional)");
        System.err.println("INF: -n=<NUM>  create subdirectories to contain <NUM> FOX files (optional)");
        System.err.println("INF: -v        validate the FOX files (optional)");
        System.err.println("INF: -l        lax check if a local resource exists (optional)");
    }

    public static void main(String[] args) {
        File   rfile = null;
        String dir = ".";
        String fdir = null;
        String idir = null;
        boolean validateFOX = false;
        boolean laxResourceCheck = false;
        int ndir = 0;
        // check command line
        OptionParser parser = new OptionParser( "lvr:f:i:n:?*" );
        OptionSet options = parser.parse(args);
        if (options.has("l"))
            laxResourceCheck = true;
        if (options.has("v"))
            validateFOX = true;
        if (options.has("r"))
            rfile = new File((String)options.valueOf("r"));
        if (options.has("f"))
            fdir = (String)options.valueOf("f");
        if (options.has("i"))
            idir = (String)options.valueOf("i");
        if (options.has("n")) {
            try {
                ndir = Integer.parseInt((String)options.valueOf("n"));
            } catch(NumberFormatException e) {
                System.err.println("FTL: -n expects a numeric argument!");
                showHelp();
                System.exit(1);
            }
        }
        if (options.has("?")) {
            showHelp();
            System.exit(0);
        }
        
        List arg = options.nonOptionArguments();
        if (arg.size()>1) {
            System.err.println("FTL: only one source <DIR> argument is allowed!");
            showHelp();
            System.exit(1);
        }
        if (arg.size() == 1)
            dir = (String)arg.get(0);

        try {
            SaxonExtensionFunctions.registerAll(SaxonUtils.getProcessor().getUnderlyingConfiguration());
        } catch (Exception e) {
            System.err.println("ERR: couldn't register the Saxon extension functions: "+e);
            e.printStackTrace();
        }
        try {
            if (fdir == null)
                fdir = dir + "/fox";
            XdmNode relsDoc = null;
            if (rfile != null && rfile.exists()) {
                relsDoc = SaxonUtils.buildDocument(new StreamSource(rfile));
                System.err.println("DBG: loaded["+rfile.getAbsolutePath()+"]");
            } else {
                // create lookup document for relations
                XsltTransformer rels = SaxonUtils.buildTransformer(Main.class.getResource("/cmd2rels.xsl")).load();
                rels.setParameter(new QName("dir"), new XdmAtomicValue("file:"+dir));
                rels.setSource(new StreamSource(Main.class.getResource("/null.xml").toString()));
                XdmDestination dest = new XdmDestination();
                rels.setDestination(dest);
                rels.transform();
                relsDoc = dest.getXdmNode();
                if (rfile != null) {
                    TransformerFactory.newInstance().newTransformer().transform(relsDoc.asSource(),new StreamResult(rfile));
                    System.err.println("DBG: saved["+rfile.getAbsolutePath()+"]");
                }
            }
            // Check the relations
            XsltTransformer rcheck = SaxonUtils.buildTransformer(Main.class.getResource("/checkRels.xsl")).load();
            rcheck.setParameter(new QName("rels-doc"), relsDoc);
            rcheck.setSource(new StreamSource(Main.class.getResource("/null.xml").toString()));
            XdmDestination dest = new XdmDestination();
            rcheck.setDestination(dest);
            rcheck.transform();
            // CMDI 2 FOX
            // create the fox dir
            FileUtils.forceMkdir(new File(fdir));
            Collection<File> inputs = FileUtils.listFiles(new File(dir),new String[] {"cmdi"},true);
            XsltExecutable cmd2fox = SaxonUtils.buildTransformer(Main.class.getResource("/cmd2fox.xsl"));
            int i = 0;
            for (File input:inputs) {
                if (!input.isHidden() && !input.getAbsolutePath().matches(".*/(corpman|sessions)/.*")) {
                    try {
                        XsltTransformer fox = cmd2fox.load();
                        //fox.setParameter(new QName("rels-uri"), new XdmAtomicValue("file:"+map.getAbsolutePath()));
                        fox.setParameter(new QName("rels-doc"), relsDoc);
                        fox.setParameter(new QName("conversion-base"), new XdmAtomicValue(dir));
                        if (idir != null)
                            fox.setParameter(new QName("import-base"), new XdmAtomicValue(idir));
                        fox.setParameter(new QName("fox-base"), new XdmAtomicValue(fdir));
                        fox.setParameter(new QName("lax-resource-check"),new XdmAtomicValue(laxResourceCheck));
                        fox.setSource(new StreamSource(input));
                        XdmDestination destination = new XdmDestination();
                        fox.setDestination(destination);
                        fox.transform();
                        File out = new File(fdir + "/lat-"+(++i)+".xml");
                        TransformerFactory.newInstance().newTransformer().transform(destination.getXdmNode().asSource(),new StreamResult(out));
                        System.err.println("DBG: created["+out.getAbsolutePath()+"]");
                    } catch (Exception e) {
                        System.err.println("ERR: "+e);
                        System.err.println("WRN: skipping file["+input.getAbsolutePath()+"]");
                    }
                }
            }
            if (ndir > 0) {
                int n = 0;
                int d = 0;
                for (File input:FileUtils.listFiles(new File(fdir),new String[] {"xml"},true)) {
                    if (n == ndir)
                        n = 0;
                    n++;
                    FileUtils.moveFileToDirectory(input,new File(fdir+"/"+(n==1?d++:d)),true);
                }
            }
            if (validateFOX) {
                SchemAnon tron = new SchemAnon(Main.class.getResource("/foxml1-1.xsd"),"ingest");
                for (File input:FileUtils.listFiles(new File(fdir),new String[] {"xml"},true)) {
                    // validate FOX
                    if (!tron.validate(input)) {
                        System.err.println("ERR: invalid file["+input.getAbsolutePath()+"]");
                        for (Message msg : tron.getMessages()) {
                            System.out.println("" + (msg.isError() ? "ERR: " : "WRN: ") + (msg.getLocation() != null ? "at " + msg.getLocation() : ""));
                            System.out.println("" + (msg.isError() ? "ERR: " : "WRN: ") + msg.getText());
                        }
                    } else
                        System.err.println("DBG: valid file["+input.getAbsolutePath()+"]");
                }
            }
        } catch(Exception ex) {
            System.err.println("FATAL: "+ex);
            ex.printStackTrace(System.err);
        }
    }
}